/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Area52BulwarkRules.h"
#include "AscensionFreePick.h"
#include "Config.h"
#include "Containers.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"

namespace
{
bool Enabled = false;

bool Applies(Unit* unit)
{
    return unit && AscensionFreePick::Applies(unit->ToPlayer());
}

void ClearBulwark(Unit* unit)
{
    for (uint32 id : {923923u, 923924u, 923927u})
        unit->RemoveAurasDueToSpell(id);
}
}

class area52_bulwark_world final : public WorldScript
{
public:
    area52_bulwark_world() : WorldScript("area52_bulwark_world") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

class area52_bulwark_metadata final : public GlobalScript
{
public:
    area52_bulwark_metadata() : GlobalScript("area52_bulwark_metadata") { }
    bool OnIsAffectedBySpellModCheck(SpellInfo const*, SpellInfo const* spell, SpellModifier const* mod) override
    {
        return !(Enabled && mod->spellId == 920127 && mod->op == SPELLMOD_CHANCE_OF_SUCCESS &&
            (spell->Id == 20127 || spell->Id == 20130 || spell->Id == 20135));
    }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        if (info->Id == 1495 || info->Id == 14269 || info->Id == 14270 ||
            info->Id == 14271 || info->Id == 36916 || info->Id == 53339)
            info->UseRangedAttackPowerForDamage = true;
        if (info->Id == 7376)
            info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_THREAT;
        if (info->Id == 71)
            info->Effects[EFFECT_1].BasePoints = -6;
        if (info->Id == 59088 || info->Id == 59089)
            info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN;
        if (info->Id == 59088)
            info->Effects[EFFECT_0].MiscValue = SPELLMOD_COOLDOWN;
        if (info->Id == 86378)
            info->Effects[EFFECT_1].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
        if (info->Id == 923921 || info->Id == 923922)
        {
            info->AttributesEx3 |= SPELL_ATTR3_SUPPRESS_CASTER_PROCS;
            info->AttributesEx4 |= SPELL_ATTR4_SUPPRESS_WEAPON_PROCS;
        }
    }
};

class aura_area52_avatar_movement : public AuraScript
{
    PrepareAuraScript(aura_area52_avatar_movement);
    bool Load() override { return Applies(GetUnitOwner()); }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
        GetTarget()->RemoveAurasWithMechanic(1ULL << MECHANIC_ROOT);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->ApplySpellImmune(GetId(), IMMUNITY_MECHANIC, MECHANIC_ROOT, false);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_avatar_movement::Apply,
            EFFECT_2, SPELL_AURA_MOD_MINIMUM_SPEED, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_avatar_movement::Remove,
            EFFECT_2, SPELL_AURA_MOD_MINIMUM_SPEED, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_tank_stance : public AuraScript
{
    PrepareAuraScript(aura_area52_tank_stance);
    bool Load() override { return Applies(GetUnitOwner()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({991293}); }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        for (auto stance : Area52Bulwark::TankStances)
            if (stance != GetId())
                owner->RemoveAurasDueToSpell(stance);
        owner->CastSpell(owner, 991293, true);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        if (std::none_of(Area52Bulwark::TankStances.begin(), Area52Bulwark::TankStances.end(),
            [owner, this](uint32 id) { return id != GetId() && owner->HasAura(id); }))
            owner->RemoveAurasDueToSpell(991293);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_tank_stance::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_tank_stance::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_shield_tools : public SpellScript
{
    PrepareSpellScript(spell_area52_shield_tools);

    bool Load() override { return Applies(GetCaster()); }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({2565, 23922, 859090, 986126});
    }

    void Handle()
    {
        Player* player = GetCaster()->ToPlayer();
        uint32 root = GetSpellInfo()->GetFirstRankSpell()->Id;
        if ((root == 2565 || root == 23920) && player->HasAura(859089))
            player->CastSpell(player, 859090, true);
        if (!player->HasAura(986125))
            return;
        if (root == 871)
            player->RemoveSpellCooldown(2565, true);
        else if (root == 2565)
        {
            for (uint32 rank = 23922; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
                player->RemoveSpellCooldown(rank, true);
        }
        else if (root == 23922)
            player->CastSpell(player, 986126, true);
    }

    void Register() override { AfterCast += SpellCastFn(spell_area52_shield_tools::Handle); }
};

class aura_area52_bulwark_cleanup : public AuraScript
{
    PrepareAuraScript(aura_area52_bulwark_cleanup);
    bool Load() override { return Applies(GetUnitOwner()); }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetId() == 859089)
            GetTarget()->RemoveAurasDueToSpell(859090);
        else if (GetId() == 986125)
            GetTarget()->RemoveAurasDueToSpell(986126);
        else if (GetId() == 977828)
            GetTarget()->RemoveAurasDueToSpell(977829);
        else if (GetId() == 901144)
            GetTarget()->RemoveAurasDueToSpell(901145);
        else if (GetId() == 978724)
        {
            GetTarget()->RemoveAurasDueToSpell(922565);
            ClearBulwark(GetTarget());
        }
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_bulwark_cleanup::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_wild_aggression : public AuraScript
{
    PrepareAuraScript(aura_area52_wild_aggression);
    bool Load() override { return Applies(GetUnitOwner()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({977830, 977829}); }

    bool Check(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        DamageInfo* damage = event.GetDamageInfo();
        return spell && damage && Area52Bulwark::WildAggressionEligible(
            spell->GetFirstRankSpell()->Id, event.GetActor() == GetTarget(), damage->GetDamage());
    }

    void Proc(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        owner->CastSpell(owner, 977830, true, nullptr, effect);
        if (owner->GetPower(POWER_ENERGY) == owner->GetMaxPower(POWER_ENERGY))
            owner->CastSpell(owner, 977829, true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_wild_aggression::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_wild_aggression::Proc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class aura_area52_bulwark_energy : public AuraScript
{
    PrepareAuraScript(aura_area52_bulwark_energy);
    bool Load() override { return Applies(GetUnitOwner()); }

    bool Check(ProcEventInfo& event)
    {
        return event.GetActionTarget() == GetTarget() && (event.GetHitMask() & PROC_HIT_BLOCK);
    }

    void Proc(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->ModifyPower(POWER_ENERGY, 5);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_bulwark_energy::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_bulwark_energy::Proc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_area52_bulwark_channel : public SpellScript
{
    PrepareSpellScript(spell_area52_bulwark_channel);
    bool Load() override { return Applies(GetCaster()); }

    SpellCastResult Check()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player->HasAura(978724) || !player->HasActiveSpell(2565))
            return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
        if (!player->GetShield())
            return SPELL_FAILED_EQUIPPED_ITEM_CLASS;
        if (player->GetPower(POWER_ENERGY) < player->GetMaxPower(POWER_ENERGY))
            return SPELL_FAILED_NO_POWER;
        return SPELL_CAST_OK;
    }

    void Register() override { OnCheckCast += SpellCheckCastFn(spell_area52_bulwark_channel::Check); }
};

class aura_area52_bulwark_channel : public AuraScript
{
    PrepareAuraScript(aura_area52_bulwark_channel);
    bool Load() override { return Applies(GetUnitOwner()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({923923, 923927, 923921}); }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->CastSpell(GetTarget(), 923923, true);
        GetTarget()->CastSpell(GetTarget(), 923927, true);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes) { ClearBulwark(GetTarget()); }

    bool Check(ProcEventInfo& event)
    {
        Unit* attacker = event.GetActor();
        return attacker && attacker != GetTarget() && event.GetActionTarget() == GetTarget() &&
            GetTarget()->IsValidAttackTarget(attacker) && (event.GetHitMask() &
            (PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_BLOCK | PROC_HIT_ABSORB));
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(event.GetActor(), 923921, true, nullptr, effect);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_bulwark_channel::Apply,
            EFFECT_0, SPELL_AURA_MOD_BLOCK_PERCENT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_bulwark_channel::Remove,
            EFFECT_0, SPELL_AURA_MOD_BLOCK_PERCENT, AURA_EFFECT_HANDLE_REAL);
        DoCheckProc += AuraCheckProcFn(aura_area52_bulwark_channel::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_bulwark_channel::Proc, EFFECT_0, SPELL_AURA_MOD_BLOCK_PERCENT);
    }
};

class aura_area52_bulwark_drain : public AuraScript
{
    PrepareAuraScript(aura_area52_bulwark_drain);
    uint32 ticks = 0;
    bool Load() override { return Applies(GetUnitOwner()); }

    void Tick(AuraEffect const*)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        if (!owner->HasAura(922565))
        {
            Remove();
            return;
        }
        uint32 const cost = Area52Bulwark::EnergyDrain(ticks++);
        uint32 const available = owner->GetPower(POWER_ENERGY);
        owner->ModifyPower(POWER_ENERGY, -int32(std::min(cost, available)));
        if (available <= cost)
        {
            owner->InterruptSpell(CURRENT_CHANNELED_SPELL);
            owner->RemoveAurasDueToSpell(922565);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_bulwark_drain::Tick,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_area52_shockwave : public SpellScript
{
    PrepareSpellScript(spell_area52_shockwave);
    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({846968, 846969}); }

    void Suppress(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
    }

    void Trigger() { GetCaster()->CastSpell(GetCaster(), GetCaster()->HasAura(507903) ? 846969 : 846968, true); }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_area52_shockwave::Suppress, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunch += SpellEffectFn(spell_area52_shockwave::Suppress, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_shockwave::Suppress, EFFECT_0, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_shockwave::Suppress, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        AfterCast += SpellCastFn(spell_area52_shockwave::Trigger);
    }
};

class spell_area52_shockwave_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_shockwave_damage);
    bool Load() override { return Applies(GetCaster()); }

    void Damage(SpellEffIndex)
    {
        int32 const percent = GetSpellInfo()->Effects[EFFECT_2].CalcValue(GetCaster());
        SetEffectValue(GetEffectValue() + int32(CalculatePct(GetCaster()->GetTotalAttackPowerValue(BASE_ATTACK), percent)));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_shockwave_damage::Damage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_area52_bulwark_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_bulwark_damage);
    bool Load() override { return Applies(GetCaster()); }

    void Damage(SpellEffIndex index)
    {
        if (GetSpellInfo()->GetCategory() == 1209)
            return;
        uint8 const level = GetCaster()->GetLevel();
        float const limit = GetCaster()->HasAura(2565) ? 2.0f : 1.0f;
        uint32 const block = GetCaster()->GetShieldBlockValue(uint32(level * 24.5f * limit),
            uint32(level * 34.5f * limit));
        SetEffectValue(GetEffectValue() + int32(GetCaster()->ApplyEffectModifiers(GetSpellInfo(), index, float(block))));
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_bulwark_damage::Damage, EFFECT_1, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_area52_intervene_taunt : public SpellScript
{
    PrepareSpellScript(spell_area52_intervene_taunt);
    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({277421}); }

    void Taunt(SpellEffIndex)
    {
        Unit* ally = GetHitUnit();
        if (!ally)
            return;
        std::vector<Unit*> attackers;
        for (Unit* attacker : ally->getAttackers())
            if (attacker->IsAlive() && GetCaster()->IsValidAttackTarget(attacker))
                attackers.push_back(attacker);
        Acore::Containers::RandomResize(attackers, 3);
        for (Unit* attacker : attackers)
            GetCaster()->CastSpell(attacker, 277421, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_intervene_taunt::Taunt, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
    }
};

class spell_area52_primal_precision : public SpellScript
{
    PrepareSpellScript(spell_area52_primal_precision);
    bool Load() override { return Applies(GetCaster()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({48414}); }

    void Cast()
    {
        if (GetCaster()->HasAura(48410))
            GetCaster()->CastSpell(GetCaster(), 48414, true);
    }

    void Register() override { AfterCast += SpellCastFn(spell_area52_primal_precision::Cast); }
};

class aura_area52_combat_readiness : public AuraScript
{
    PrepareAuraScript(aura_area52_combat_readiness);
    bool Load() override { return Applies(GetUnitOwner()); }

    bool Check(ProcEventInfo& event)
    {
        return event.GetActionTarget() == GetTarget() && event.GetDamageInfo() &&
            event.GetActor() != GetTarget() && (event.GetHitMask() &
            (PROC_HIT_NORMAL | PROC_HIT_CRITICAL | PROC_HIT_BLOCK | PROC_HIT_ABSORB));
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(86379);
        GetTarget()->RemoveAurasDueToSpell(GetId() == 86378 ? 886378 : 86378);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_combat_readiness::Check);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_combat_readiness::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddAscensionBulwarkScripts()
{
    new area52_bulwark_world();
    new area52_bulwark_metadata();
    RegisterSpellScript(aura_area52_tank_stance);
    RegisterSpellScript(aura_area52_avatar_movement);
    RegisterSpellScript(spell_area52_shield_tools);
    RegisterSpellScript(aura_area52_bulwark_cleanup);
    RegisterSpellScript(aura_area52_wild_aggression);
    RegisterSpellScript(aura_area52_bulwark_energy);
    RegisterSpellAndAuraScriptPair(spell_area52_bulwark_channel, aura_area52_bulwark_channel);
    RegisterSpellScript(aura_area52_bulwark_drain);
    RegisterSpellScript(spell_area52_shockwave);
    RegisterSpellScript(spell_area52_shockwave_damage);
    RegisterSpellScript(spell_area52_bulwark_damage);
    RegisterSpellScript(spell_area52_intervene_taunt);
    RegisterSpellScript(spell_area52_primal_precision);
    RegisterSpellScript(aura_area52_combat_readiness);
}
