/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "AscensionMysticEnchantCatalog.h"
#include "MotionMaster.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "WorldSession.h"
#include <algorithm>
#include <array>

namespace
{
bool Enabled = false;
constexpr uint32 Light = 926573, Hook = 92580, Deconstruction = 954081, GhostlyBlow = 983240;
constexpr std::array<uint32, 8> Consecration = {26573, 20116, 20922, 20923, 20924, 27173, 48818, 48819};
constexpr std::array<uint32, 9> Devastate = {20243, 302035, 302036, 302037, 302038, 30016, 30022, 47497, 47498};
constexpr std::array<uint32, 9> Deconstruct = {954082, 954443, 954444, 954445, 954446, 954447, 954448, 954449, 954450};
bool Applies(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}
int32 DeconstructionDuration(uint32 stacks)
{
    return 3000 * std::min<uint32>(stacks, 5);
}
void Replacement(Player* player, uint32 original, uint32 replacement, bool active, uint32 enchant)
{
    AscensionMysticEnchant::SetGrantedSpell(player, enchant, replacement, active && player->HasSpell(original));
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
void Synchronize(Player* player)
{
    if (!player || !player->GetSession() || player->GetSession()->PlayerLogout())
        return;
    bool light = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Light) && !player->HasAura(81315);
    if (!light && Applies(player))
    {
        player->RemoveAurasDueToSpell(926590, player->GetGUID());
        for (uint32 id = 926574; id <= 926588; id += 2)
            player->RemoveAurasDueToSpell(id, player->GetGUID());
    }
    bool deconstruct = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Deconstruction);
    bool hook = Applies(player) && AscensionMysticEnchant::GrantsActive(player, Hook);
    for (std::size_t i = 0; i < Consecration.size(); ++i)
        Replacement(player, Consecration[i], 926574 + uint32(i) * 2, light, Light);
    for (std::size_t i = 0; i < Devastate.size(); ++i)
        Replacement(player, Devastate[i], Deconstruct[i], deconstruct, Deconstruction);
    AscensionMysticEnchant::SetGrantedSpell(player, Hook, 92583, hook);
    Replacement(player, 14251, 983241,
        Applies(player) && AscensionMysticEnchant::GrantsActive(player, GhostlyBlow), GhostlyBlow);
}
class EpicWorld final : public WorldScript
{
public:
    EpicWorld() : WorldScript("Area52EpicWorld") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};
class EpicContracts final : public GlobalScript
{
public:
    EpicContracts() : GlobalScript("Area52EpicContracts") { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        if (info->Id == 926590)
            info->Effects[1].BasePoints = -2;
        if (info->Id >= 926575 && info->Id <= 926589 && (info->Id % 2) == 1)
            info->Effects[0].BonusMultiplier = 0.0264f;
        if (info->Id == 92583)
            info->Effects[0].BonusMultiplier = 0.2f;
        auto const rank = std::find(Deconstruct.begin(), Deconstruct.end(), info->Id);
        if (rank != Deconstruct.end())
        {
            constexpr std::array<int32, 9> points = {14, 20, 29, 43, 64, 95, 140, 208, 309};
            info->Effects[1].BasePoints = points[rank - Deconstruct.begin()];
            info->Effects[1].BonusMultiplier = 0.3f;
            info->Effects[2].BasePoints = -1;
        }
    }
};
class EpicPlayer final : public PlayerScript
{
public:
    EpicPlayer() : PlayerScript("Area52EpicPlayer") { }
    void Refresh(Player* player, uint32 spell)
    {
        if (Applies(player) && (spell == 14251 || std::find(Consecration.begin(), Consecration.end(), spell) != Consecration.end() ||
            std::find(Devastate.begin(), Devastate.end(), spell) != Devastate.end()))
            Synchronize(player);
    }
    void OnPlayerLearnSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
};
class aura_area52_epic_equipment : public AuraScript
{
    PrepareAuraScript(aura_area52_epic_equipment);
    bool Load() override { return GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({92583, 9654556, 926590, 954083, 58567}) &&
            ValidateSpellInfo(Consecration) && ValidateSpellInfo(Devastate) && ValidateSpellInfo(Deconstruct);
    }
    void Apply(AuraEffect const*, AuraEffectHandleModes) { Synchronize(GetTarget()->ToPlayer()); }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        Synchronize(player);
        if (GetId() == Light)
        {
            player->RemoveAurasDueToSpell(926590, player->GetGUID());
            for (uint32 id = 926574; id <= 926588; id += 2)
                player->RemoveAurasDueToSpell(id, player->GetGUID());
        }
        if (GetId() == Deconstruction)
            player->RemoveAurasDueToSpell(954083, player->GetGUID());
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_epic_equipment::Apply, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_epic_equipment::Remove, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};
class spell_area52_emanate_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_emanate_damage);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Light); }
    void Hit()
    {
        if (GetHitDamage() > 0 && !GetCaster()->HasAura(81315))
            GetCaster()->CastSpell(GetCaster(), 926590, true);
    }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.022f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_emanate_damage::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_emanate_damage::Hit);
    }
};
class spell_area52_flesh_hook : public SpellScript
{
    PrepareSpellScript(spell_area52_flesh_hook);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Hook); }
    SpellCastResult Check()
    {
        Unit* target = GetExplTargetUnit();
        return target && target->IsPlayer() ? SPELL_FAILED_BAD_TARGETS : SPELL_CAST_OK;
    }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.2f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_flesh_hook::Check);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_flesh_hook::Damage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
class spell_area52_flesh_pull : public SpellScript
{
    PrepareSpellScript(spell_area52_flesh_pull);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Hook); }
    void Pull(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* target = GetHitUnit();
        if (!target || target->IsPlayer() || !target->IsAlive() || target->HasUnitState(UNIT_STATE_ROOT))
            return;
        Unit* caster = GetCaster();
        float speed = 50.0f;
        float vertical = target->GetDistance(caster) / speed * 0.5f * 19.291105f;
        target->GetMotionMaster()->MoveJump(caster->GetPositionX(), caster->GetPositionY(),
            caster->GetPositionZ() + 1.0f, speed, vertical);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_flesh_pull::Pull, EFFECT_0, SPELL_EFFECT_PULL_TOWARDS_DEST);
    }
};
class spell_area52_deconstruct : public SpellScript
{
    PrepareSpellScript(spell_area52_deconstruct);
    bool Load() override { return Applies(GetCaster()->ToPlayer()) && GetCaster()->HasAura(Deconstruction); }
    void Damage(SpellEffIndex)
    {
        SetEffectValue(GetEffectValue() + int32(0.195f * GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK)));
    }
    void Hit()
    {
        Unit* target = GetHitUnit();
        if (!target || GetHitDamage() <= 0)
            return;
        if (Aura* sunder = target->GetAura(58567))
            if (Aura* buff = GetCaster()->AddAura(954083, GetCaster()))
            {
                int32 duration = DeconstructionDuration(sunder->GetStackAmount());
                buff->SetMaxDuration(duration);
                buff->SetDuration(duration);
            }
    }
    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_deconstruct::Damage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
        AfterHit += SpellHitFn(spell_area52_deconstruct::Hit);
    }
};
class aura_area52_deconstruction_power : public AuraScript
{
    PrepareAuraScript(aura_area52_deconstruction_power);
    bool Load() override { return GetUnitOwner() && Applies(GetUnitOwner()->ToPlayer()); }
    void Amount(AuraEffect const*, int32& amount, bool&)
    {
        amount = int32(GetUnitOwner()->GetLevel() * 1.2f);
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_deconstruction_power::Amount,
            EFFECT_1, SPELL_AURA_ASCENSION_MOD_SPELL_POWER_FLAT);
    }
};

class aura_area52_ghostly_blow : public AuraScript
{
    PrepareAuraScript(aura_area52_ghostly_blow);
    bool Load() override { return GetCaster() && Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({983240, 983241, 983242, 14251}); }
    void Tick(AuraEffect const*)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || !player->IsAlive() || !target->IsAlive() ||
            !AscensionMysticEnchant::GrantsActive(player, GhostlyBlow) || !player->HasSpell(14251))
        {
            PreventDefaultAction();
            Remove();
            return;
        }
        if (!player->IsWithinDistInMap(target, GetSpellInfo()->GetMaxRange(false)))
        {
            PreventDefaultAction();
            return;
        }
        GetAura()->RefreshDuration();
    }
    void Refresh(AuraEffect const*) { PreventDefaultAction(); }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_ghostly_blow::Tick,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_ghostly_blow::Refresh,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};
class ghostly_blow_damage : public UnitScript
{
public:
    ghostly_blow_damage() : UnitScript("ghostly_blow_damage", true, {UNITHOOK_ON_PERIODIC_DAMAGE_RESULT}) { }
    void OnPeriodicDamageResult(Unit*, Unit* caster, uint32 damage, SpellInfo const* info) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!damage || !info || info->Id != 983242 || !Applies(player) || !player->IsAlive() ||
            !AscensionMysticEnchant::GrantsActive(player, GhostlyBlow))
            return;
        if (player->HasAura(13713) || player->HasAura(13853) || player->HasAura(13854))
            player->ModifySpellCooldown(14278, -1000);
    }
};
}
void AddAscensionMysticKnightEpicScripts()
{
    new EpicWorld();
    new EpicContracts();
    new EpicPlayer();
    new ghostly_blow_damage();
    RegisterSpellScript(aura_area52_ghostly_blow);
    RegisterSpellScript(aura_area52_epic_equipment);
    RegisterSpellScript(spell_area52_emanate_damage);
    RegisterSpellScript(spell_area52_flesh_hook);
    RegisterSpellScript(spell_area52_flesh_pull);
    RegisterSpellScript(spell_area52_deconstruct);
    RegisterSpellScript(aura_area52_deconstruction_power);
}

bool HasArea52EpicEquipmentHandlers(uint32 spell)
{
    if (!Enabled || (spell != Light && spell != Hook && spell != Deconstruction && spell != GhostlyBlow))
        return false;
    std::list<AuraScript*> auras;
    sScriptMgr->CreateAuraScripts(spell, auras);
    bool found = false;
    for (auto* script : auras)
    {
        found |= *script->_GetScriptName() == "aura_area52_epic_equipment";
        delete script;
    }
    if (!found)
        return false;
    std::vector<std::pair<uint32, std::string>> required;
    if (spell == Hook)
        required = {{92583, "spell_area52_flesh_hook"}, {9654556, "spell_area52_flesh_pull"}};
    if (spell == Light)
        for (uint32 id = 926575; id <= 926589; id += 2)
            required.emplace_back(id, "spell_area52_emanate_damage");
    if (spell == Deconstruction)
        for (uint32 id : Deconstruct)
        {
            required.emplace_back(id, "spell_area52_deconstruct");
            required.emplace_back(id, "spell_area52_blademaster_empower");
        }
    for (auto const& [id, name] : required)
    {
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(id, scripts);
        found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == name;
            delete script;
        }
        if (!found)
            return false;
    }
    if (spell == Deconstruction || spell == GhostlyBlow)
    {
        std::list<AuraScript*> helpers;
        sScriptMgr->CreateAuraScripts(spell == GhostlyBlow ? 983242 : 954083, helpers);
        found = false;
        for (auto* script : helpers)
        {
            found |= *script->_GetScriptName() == (spell == GhostlyBlow ? "aura_area52_ghostly_blow" :
                "aura_area52_deconstruction_power");
            delete script;
        }
        if (!found)
            return false;
    }
    return true;
}

void AscensionMysticEnchant::RefreshEpicReplacements(Player* player)
{
    if (Applies(player))
        Synchronize(player);
}
