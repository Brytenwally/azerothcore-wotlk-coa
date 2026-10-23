/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "AscensionMysticEnchantCatalog.h"
#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Timer.h"
#include "WorldSession.h"
#include <algorithm>
#include <array>

namespace
{
constexpr uint32 Enchant = 81116;
constexpr uint32 Weapon = 983701;
constexpr uint32 Effusion = 983702;
constexpr uint32 Knight = 983715;
constexpr uint32 Horror = 983722;
constexpr uint32 Anomaly = 983723;
constexpr std::array<uint32, 5> HolyWrath = {2812, 10318, 27139, 48816, 48817};
bool Enabled = false;

bool Applies(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

void GrantAnomaly(Player* player)
{
    player->CastSpell(player, Anomaly, true);
    Aura* anomaly = player->GetAura(Anomaly, player->GetGUID());
    if (anomaly && anomaly->GetStackAmount() >= anomaly->GetSpellInfo()->StackAmount)
    {
        player->RemoveAurasDueToSpell(Anomaly, player->GetGUID());
        player->CastSpell(player, Horror, true);
    }
}

void Synchronize(Player* player)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout())
        return;
    bool const active = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Enchant);
    AscensionMysticEnchant::SetGrantedSpell(player, Enchant, Effusion, active);
    for (std::size_t i = 0; i < HolyWrath.size(); ++i)
    {
        uint32 const original = HolyWrath[i];
        uint32 const replacement = 983703 + i;
        AscensionMysticEnchant::SetGrantedSpell(player, Enchant, replacement, active && player->HasSpell(original));
        if (active && player->HasActiveSpell(original))
        {
            player->SetTemporarySpellReplacement(original, replacement);
        }
        else
        {
            if (player->GetTemporarySpellReplacement(original) == replacement)
                player->SetTemporarySpellReplacement(original, 0);
        }
    }
}

class EldritchRealm final : public WorldScript
{
public:
    EldritchRealm() : WorldScript("Area52EldritchRealm") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

class EldritchPlayer final : public PlayerScript
{
public:
    EldritchPlayer() : PlayerScript("Area52EldritchPlayer") { }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override
    {
        if (Applies(player) && std::find(HolyWrath.begin(), HolyWrath.end(), spell) != HolyWrath.end())
            Synchronize(player);
    }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override
    {
        if (Applies(player) && std::find(HolyWrath.begin(), HolyWrath.end(), spell) != HolyWrath.end())
            Synchronize(player);
    }
};

class aura_area52_eldritch_knight : public AuraScript
{
    PrepareAuraScript(aura_area52_eldritch_knight);
    uint32 LastExplosion = 0;
    bool Exploded = false;

    bool Load() override { return GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Weapon, Effusion, Knight, Horror, Anomaly, 983708, 983710,
            983711, 983712, 983718, 983721, 983724, 983703, 983704, 983705, 983706, 983707});
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Synchronize(GetTarget()->ToPlayer());
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        Synchronize(player);
        for (uint32 spell : {Weapon, Knight, Horror, Anomaly})
            player->RemoveAurasDueToSpell(spell, player->GetGUID());
        if (Item* weapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND))
            if (weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 945)
            {
                player->ApplyEnchantment(weapon, TEMP_ENCHANTMENT_SLOT, false);
                weapon->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
            }
    }
    bool Check(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        DamageInfo const* damage = event.GetDamageInfo();
        if (!Applies(player) || !player->IsAlive() || !player->IsInWorld() || event.GetActor() != player ||
            !damage || !damage->GetDamage() || !event.GetActionTarget() || event.GetActionTarget() == player)
            return false;
        if (Spell const* spell = event.GetProcSpell())
            return !spell->IsTriggered() && !spell->GetScriptValue(Enchant) &&
                (event.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) &&
                (player->HasAura(Weapon) || player->HasAura(Knight) || player->HasAura(Horror));
        return (event.GetTypeMask() & PROC_FLAG_DONE_MELEE_AUTO_ATTACK) && player->HasAura(Horror);
    }
    void Proc(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (!Check(event))
            return;
        Player* player = GetTarget()->ToPlayer();
        Unit* target = event.GetActionTarget();
        if (Spell const* spell = event.GetProcSpell())
        {
            const_cast<Spell*>(spell)->SetScriptValue(Enchant, 1);
            if (player->HasAura(Weapon))
            {
                player->CastSpell(player, Knight, true);
                uint32 const now = getMSTime();
                if (!Exploded || getMSTimeDiff(LastExplosion, now) >= 3000)
                {
                    LastExplosion = now;
                    Exploded = true;
                    player->CastSpell(target, 983711, true);
                }
            }
            if (player->HasAura(Knight))
            {
                player->CastSpell(player, 983712, true);
                GrantAnomaly(player);
            }
        }
        if (player->HasAura(Horror))
        {
            player->CastSpell(player, 983718, true);
            player->CastSpell(target, 983721, true);
        }
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_eldritch_knight::Apply,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_eldritch_knight::Remove,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoCheckProc += AuraCheckProcFn(aura_area52_eldritch_knight::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_eldritch_knight::Proc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_area52_eldritch_missing_mana : public SpellScript
{
    PrepareSpellScript(spell_area52_eldritch_missing_mana);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Enchant); }
    void Restore(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* target = GetHitUnit();
        if (!target || target != GetCaster() || !target->IsAlive() || target->HasUnitState(UNIT_STATE_ISOLATED))
            return;
        uint32 const missing = target->GetMaxPower(POWER_MANA) - target->GetPower(POWER_MANA);
        uint32 const gain = uint64(missing) * std::clamp(GetEffectValue(), 0, 100) / 100;
        GetCaster()->EnergizeBySpell(target, GetSpellInfo()->Id, gain, POWER_MANA);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_eldritch_missing_mana::Restore,
            EFFECT_0, SPELL_EFFECT_ENERGIZE_PCT);
    }
};

class spell_area52_eldritch_condemnation : public SpellScript
{
    PrepareSpellScript(spell_area52_eldritch_condemnation);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Enchant); }
    void GrantWrathAnomaly()
    {
        if (Player* player = GetCaster()->ToPlayer())
            GrantAnomaly(player);
    }
    void Condemn(SpellEffIndex)
    {
        if (Unit* target = GetHitUnit())
            if (target != GetCaster() && GetHitDamage() > 0)
                GetCaster()->CastSpell(target, 983710, true);
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_eldritch_condemnation::GrantWrathAnomaly);
        OnEffectHitTarget += SpellEffectFn(spell_area52_eldritch_condemnation::Condemn,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddAscensionEldritchKnightScripts()
{
    new EldritchRealm();
    new EldritchPlayer();
    RegisterSpellScript(aura_area52_eldritch_knight);
    RegisterSpellScript(spell_area52_eldritch_missing_mana);
    RegisterSpellScript(spell_area52_eldritch_condemnation);
}
