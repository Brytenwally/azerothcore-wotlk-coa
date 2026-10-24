/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "AscensionMysticEnchantCatalog.h"
#include "Player.h"
#include "Item.h"
#include "SpellMgr.h"
#include "Spell.h"
#include "WorldSession.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include <algorithm>

namespace
{
bool Enabled = false;
constexpr uint32 Hoplite = 978217;
constexpr uint32 SpearMastery = 901203;
constexpr char ChannelKey[] = "coa.area52.hoplite.flurry";

bool InArea52(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

bool Applies(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return InArea52(player) && player->HasAura(Hoplite);
}

void ReplaceChain(Player* player, uint32 root, uint32 replacement, bool enabled)
{
    if (sSpellMgr->GetNextSpellInChain(replacement))
    {
        for (uint32 original = root, rank = replacement; original && rank;
            original = sSpellMgr->GetNextSpellInChain(original), rank = sSpellMgr->GetNextSpellInChain(rank))
        {
            bool const known = enabled && player->HasSpell(original);
            AscensionMysticEnchant::SetGrantedSpell(player, Hoplite, rank, known);
            if (known && player->HasActiveSpell(original))
                player->SetTemporarySpellReplacement(original, rank);
            else if (uint32 const current = player->GetTemporarySpellReplacement(original);
                current && sSpellMgr->GetFirstSpellInChain(current) == replacement)
                player->SetTemporarySpellReplacement(original, 0);
        }
        return;
    }
    bool wanted = false;
    for (uint32 original = root; original; original = sSpellMgr->GetNextSpellInChain(original))
        wanted |= enabled && player->HasActiveSpell(original);
    AscensionMysticEnchant::SetGrantedSpell(player, Hoplite, replacement, wanted);
    for (uint32 original = root; original; original = sSpellMgr->GetNextSpellInChain(original))
    {
        if (wanted && player->HasActiveSpell(original))
            player->SetTemporarySpellReplacement(original, replacement);
        else if (player->GetTemporarySpellReplacement(original) == replacement)
            player->SetTemporarySpellReplacement(original, 0);
    }

}

void SynchronizeHoplite(Player* player)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout())
        return;
    bool active = AscensionMysticEnchant::GrantsActive(player, Hoplite);
    ReplaceChain(player, 23922, 978533, active);
    ReplaceChain(player, 1752, 978569, active);
    ReplaceChain(player, 78, 982250, active);
    ReplaceChain(player, 954876, 901200, active);
    if (active && player->HasAura(2457))
    {
        if (!player->HasAura(997743))
            player->AddAura(997743, player);
    }
    else
    {
        player->RemoveAurasDueToSpell(997743);
        player->RemoveAurasDueToSpell(978539);
        player->RemoveAurasDueToSpell(978537);
    }
    if (!active)
    {
        player->RemoveAurasDueToSpell(901200);
        player->RemoveAurasDueToSpell(SpearMastery);
    }
}

class aura_area52_hoplite_equipment : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_equipment);
    bool Load() override
    {
        Player* player = GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr;
        return InArea52(player);
    }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({978533, 978569, 982250, 901200, 997743});
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        SynchronizeHoplite(GetUnitOwner()->ToPlayer());
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        if (player->GetSession()->PlayerLogout())
            return;
        SynchronizeHoplite(player);
        Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (main && off && main->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM &&
            off->GetTemplate()->InventoryType == INVTYPE_SHIELD)
            player->AutoUnequipOffhandIfNeed(true);
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_hoplite_equipment::Apply,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_hoplite_equipment::Remove,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class HoplitePlayer final : public PlayerScript
{
public:
    HoplitePlayer() : PlayerScript("Area52HoplitePlayer") { }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void Refresh(Player* player, uint32 spell)
    {
        uint32 root = sSpellMgr->GetFirstSpellInChain(spell);
        if (Applies(player) && (root == 23922 || root == 1752 || root == 78 || root == 954876))
            SynchronizeHoplite(player);
    }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (Applies(player) && !player->GetSession()->PlayerLogout() &&
            player->HasAura(2457) != player->HasAura(997743))
            SynchronizeHoplite(player);
    }
};

class HopliteWorld final : public WorldScript
{
public:
    HopliteWorld() : WorldScript("Area52HopliteWorld") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

struct FlurryState : DataMap::Base
{
    bool Empowered = false;
    uint8 Points = 0;
};

int32 ShieldStrikeDamage(uint8 points, float attackPower)
{
    points = std::min<uint8>(points, 5);
    return int32(18.0f + 221.0f * points + (0.82f + 0.04f * points) * attackPower);
}

float FlurryScaling(uint8 points, float attackPower, float spellPower)
{
    points = std::min<uint8>(points, 5);
    return (attackPower + spellPower) * (0.135f + 0.027f * points * points) / 7.0f;
}

uint32 ImpaledBonus(int32 maximum, int32 remaining, bool playerTarget)
{
    uint32 seconds = uint32(std::clamp(maximum - remaining, 0, 12000)) / 1000;
    return seconds * (playerTarget ? 5u : 20u);
}

class HopliteContracts final : public GlobalScript
{
public:
    HopliteContracts() : GlobalScript("Area52HopliteContracts") { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        if (info->Id == 982250 || (info->Id >= 300804 && info->Id <= 300815))
            info->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
        if (info->Id == 901201)
        {
            info->Effects[EFFECT_0].DieSides = 1;
            info->Effects[EFFECT_0].RealPointsPerLevel = 0;
            info->Effects[EFFECT_0].BonusMultiplier = 0;
        }
    }
};

class aura_area52_hoplite_incompatible : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_incompatible);
    bool Load() override { return InArea52(GetUnitOwner()->ToPlayer()); }
    bool Check(Unit* target) { return !Applies(target); }
    void Register() override
    {
        DoCheckAreaTarget += AuraCheckAreaTargetFn(aura_area52_hoplite_incompatible::Check);
    }
};

class spell_area52_hoplite_incompatible : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_incompatible);
    bool Load() override { return InArea52(GetCaster()->ToPlayer()); }
    SpellCastResult Check() { return Applies(GetCaster()) ? SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW : SPELL_CAST_OK; }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_hoplite_incompatible::Check);
    }
};

class spell_area52_hoplite_thrust : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_thrust);
    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({29131, 978539, 978537}); }
    void Stack()
    {
        if (!GetCaster()->HasAura(29131) || !GetCaster()->HasAura(997743))
            return;
        GetCaster()->CastSpell(GetCaster(), 978539, true);
        GetCaster()->CastSpell(GetCaster(), 978537, true);
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_hoplite_thrust::Stack);
    }
};

class spell_area52_hoplite_bloodrage : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_bloodrage);
    bool Load() override { return Applies(GetCaster()); }
    void Reset()
    {
        GetCaster()->RemoveAurasDueToSpell(978539);
        GetCaster()->RemoveAurasDueToSpell(978537);
    }
    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_hoplite_bloodrage::Reset);
    }
};

class aura_area52_hoplite_phalanx_damage : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_phalanx_damage);
    bool Load() override { return Applies(GetUnitOwner()); }
    void Modifier(AuraEffect const* effect, SpellModifier*& mod)
    {
        if (!mod)
        {
            mod = new SpellModifier(GetAura());
            mod->type = SPELLMOD_PCT;
            mod->op = SPELLMOD_DAMAGE;
            mod->spellId = GetId();
            mod->mask = GetSpellInfo()->Effects[EFFECT_0].SpellClassMask;
        }
        mod->value = effect->GetAmount();
    }
    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(aura_area52_hoplite_phalanx_damage::Modifier,
            EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER);
    }
};

class spell_area52_hoplite_javelin : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_javelin);
    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({982251}); }
    void Suppress(SpellEffIndex effect) { PreventHitDefaultEffect(effect); }
    void Hit()
    {
        if (GetHitUnit() && GetHitDamage() > 0)
            GetCaster()->CastSpell(GetHitUnit(), 982251, true);
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_javelin::Suppress,
            EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        AfterHit += SpellHitFn(spell_area52_hoplite_javelin::Hit);
    }
};

class aura_area52_hoplite_impaled : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_impaled);
    bool Load() override { return Applies(GetCaster()); }
    void Tick(AuraEffect const*) { PreventDefaultAction(); }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_hoplite_impaled::Tick,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_area52_hoplite_colossus : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_colossus);
    bool Load() override { return Applies(GetCaster()); }
    bool Impaled = false;
    void Damage()
    {
        Unit* target = GetHitUnit();
        Aura* aura = target ? target->GetAura(982251, GetCaster()->GetGUID()) : nullptr;
        if (!aura || GetHitDamage() <= 0)
            return;
        Impaled = true;
        uint32 bonus = ImpaledBonus(aura->GetMaxDuration(), aura->GetDuration(),
            target->GetCharmerOrOwnerPlayerOrPlayerItself() != nullptr);
        SetHitDamage(GetHitDamage() + CalculatePct(GetHitDamage(), bonus));
    }
    void Finish()
    {
        if (!Impaled || !GetHitUnit() || GetHitDamage() <= 0)
            return;
        GetHitUnit()->RemoveAurasDueToSpell(982251, GetCaster()->GetGUID());
        for (uint32 rank = 982250; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
            GetCaster()->ToPlayer()->RemoveSpellCooldown(rank, true);
    }
    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_hoplite_colossus::Damage);
        AfterHit += SpellHitFn(spell_area52_hoplite_colossus::Finish);
    }
};

class spell_area52_hoplite_flurry : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_flurry);
    bool Load() override { return Applies(GetCaster()); }
    void Capture()
    {
        auto* state = GetCaster()->ToPlayer()->CustomData.GetDefault<FlurryState>(ChannelKey);
        state->Points = std::min<uint8>(GetCaster()->GetComboPoints(), 5);
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_area52_hoplite_flurry::Capture);
    }
};

class spell_area52_hoplite_requirements : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_requirements);
    bool Load() override { return InArea52(GetCaster()->ToPlayer()); }
    SpellCastResult Check()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!Applies(player) || !player->HasAura(2457))
            return SPELL_FAILED_ONLY_SHAPESHIFT;
        Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
        if (!main || !off || main->GetTemplate()->Class != ITEM_CLASS_WEAPON ||
            main->GetTemplate()->SubClass != ITEM_SUBCLASS_WEAPON_POLEARM ||
            off->GetTemplate()->InventoryType != INVTYPE_SHIELD)
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        return SPELL_CAST_OK;
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_hoplite_requirements::Check);
    }
};

class spell_area52_hoplite_shield_strike : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_shield_strike);
    uint8 Points = 0;
    bool Resolved = false;

    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({2687, SpearMastery}); }
    void Capture() { Points = std::min<uint8>(GetCaster()->GetComboPoints(), 5); }
    void SuppressCooldown(SpellEffIndex effect) { PreventHitDefaultEffect(effect); }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(ShieldStrikeDamage(Points, GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Hit()
    {
        if (Resolved || !Points || !GetHitUnit() || GetHitDamage() <= 0)
            return;
        Resolved = true;
        Player* player = GetCaster()->ToPlayer();
        player->ModifySpellCooldown(2687, -2000 * int32(Points));
        if (roll_chance_i(15))
            player->CastSpell(player, SpearMastery, true);
    }
    void Register() override
    {
        BeforeCast += SpellCastFn(spell_area52_hoplite_shield_strike::Capture);
        SpellInfo const* info = sSpellMgr->GetSpellInfo(m_scriptSpellId);
        if (info && info->Effects[EFFECT_0].Effect == SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE)
        {
            OnEffectHit += SpellEffectFn(spell_area52_hoplite_shield_strike::SuppressCooldown,
                EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE);
            OnEffectHitTarget += SpellEffectFn(spell_area52_hoplite_shield_strike::SuppressCooldown,
                EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE_SPELL_WITH_VALUE);
        }
        else
        {
            OnEffectLaunch += SpellEffectFn(spell_area52_hoplite_shield_strike::SuppressCooldown,
                EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE);
            OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_shield_strike::SuppressCooldown,
                EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL_WITH_VALUE);
        }
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_shield_strike::Damage,
            EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_hoplite_shield_strike::Hit);
    }
};

class aura_area52_hoplite_flurry : public AuraScript
{
    PrepareAuraScript(aura_area52_hoplite_flurry);
    bool Load() override { return Applies(GetUnitOwner()); }
    void Scale(AuraEffect const*, int32& amount, bool& recalculate)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        auto const* state = player->CustomData.Get<FlurryState>(ChannelKey);
        if (state)
            amount += int32(FlurryScaling(state->Points, player->GetTotalAttackPowerValue(BASE_ATTACK),
                float(std::max(0, player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NORMAL)))));
        recalculate = false;
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetUnitOwner()->ToPlayer();
        auto* state = player->CustomData.GetDefault<FlurryState>(ChannelKey);
        state->Empowered = player->HasAura(SpearMastery);
        if (state->Empowered)
            player->RemoveAurasDueToSpell(SpearMastery);
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetUnitOwner()->ToPlayer()->CustomData.Erase(ChannelKey);
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_hoplite_flurry::Scale,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        AfterEffectApply += AuraEffectApplyFn(aura_area52_hoplite_flurry::Apply,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_hoplite_flurry::Remove,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_hoplite_flurry_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_hoplite_flurry_damage);
    bool Load() override { return Applies(GetCaster()); }
    void Damage(SpellEffIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        auto const* state = player->CustomData.Get<FlurryState>(ChannelKey);
        Unit* target = GetHitUnit();
        if (state && state->Empowered && player->HasAura(901200) && target &&
            target->IsCreature() && !target->GetCharmerOrOwnerPlayerOrPlayerItself())
            SetEffectValue(GetEffectValue() + CalculatePct(GetEffectValue(), 50));
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_hoplite_flurry_damage::Damage,
            EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddAscensionHopliteScripts()
{
    new HopliteWorld();
    new HopliteContracts();
    RegisterSpellScript(aura_area52_hoplite_incompatible);
    RegisterSpellScript(spell_area52_hoplite_incompatible);
    RegisterSpellScript(spell_area52_hoplite_thrust);
    RegisterSpellScript(spell_area52_hoplite_bloodrage);
    RegisterSpellScript(aura_area52_hoplite_phalanx_damage);
    RegisterSpellScript(spell_area52_hoplite_javelin);
    RegisterSpellScript(aura_area52_hoplite_impaled);
    RegisterSpellScript(spell_area52_hoplite_colossus);
    RegisterSpellScript(spell_area52_hoplite_flurry);
    new HoplitePlayer();
    RegisterSpellScript(spell_area52_hoplite_requirements);
    RegisterSpellScript(aura_area52_hoplite_equipment);
    RegisterSpellScript(spell_area52_hoplite_shield_strike);
    RegisterSpellScript(aura_area52_hoplite_flurry);
    RegisterSpellScript(spell_area52_hoplite_flurry_damage);
}

bool CanArea52HopliteUseShield(Player const* player)
{
    return InArea52(player) && player->HasAura(Hoplite);
}

bool HasArea52HopliteHandlers(uint32 spell)
{
    if (spell != Hoplite || !Enabled)
        return false;
    for (auto const& [id, expected] : std::initializer_list<std::pair<uint32, char const*>>{
        {978217, "aura_area52_hoplite_equipment"}, {978539, "aura_area52_hoplite_phalanx_damage"},
        {978537, "aura_area52_hoplite_phalanx_damage"}, {982251, "aura_area52_hoplite_impaled"},
        {901200, "aura_area52_hoplite_flurry"}})
    {
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
        std::list<AuraScript*> scripts;
        sScriptMgr->CreateAuraScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == expected;
            delete script;
        }
        if (!found)
            return false;
    }
    for (auto const& [id, expected] : std::initializer_list<std::pair<uint32, char const*>>{
        {978533, "spell_area52_hoplite_shield_strike"}, {300158, "spell_area52_hoplite_shield_strike"},
        {978569, "spell_area52_hoplite_thrust"}, {2687, "spell_area52_hoplite_bloodrage"},
        {982250, "spell_area52_hoplite_javelin"}, {86361, "spell_area52_hoplite_colossus"},
        {901200, "spell_area52_hoplite_flurry"}, {901201, "spell_area52_hoplite_flurry_damage"}})
    {
        if (!sSpellMgr->GetSpellInfo(id))
            return false;
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == expected;
            delete script;
        }
        if (!found)
            return false;
    }
    return true;
}
