/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AllSpellScript.h"
#include "Area52DruidTalentRules.h"
#include "AscensionFreePick.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <list>

namespace
{
using namespace Area52DruidTalents;

Player* FreePickPlayer(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return AscensionFreePick::Applies(player) ? player : nullptr;
}

bool IsDruidSpell(SpellInfo const* info, uint32 flag0)
{
    return info && info->SpellFamilyName == SPELLFAMILY_DRUID && (info->SpellFamilyFlags[0] & flag0);
}

AuraEffect* OwnPeriodicDamage(Unit* target, Unit* caster, uint32 flag0)
{
    return target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_DRUID, flag0, 0, 0, caster->GetGUID());
}

uint32 HighestKnownRank(Player* player, uint32 firstRank)
{
    for (uint32 rank = sSpellMgr->GetLastSpellInChain(firstRank); rank; rank = sSpellMgr->GetPrevSpellInChain(rank))
        if (player->HasSpell(rank))
            return rank;
    return 0;
}

class area52_druid_balance_capstones : public AllSpellScript
{
public:
    area52_druid_balance_capstones() :
        AllSpellScript("area52_druid_balance_capstones", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo, uint32 damage, uint32,
        bool critical) override
    {
        Player* player = FreePickPlayer(spell->GetCaster());
        SpellInfo const* info = spell->GetSpellInfo();
        if (!player || !target || target == player || !info)
            return;
        bool const hit = missInfo == SPELL_MISS_NONE;
        if (IsDruidSpell(info, MoonfireFlag0))
            SpendMoonfury(player, target, hit);
        if (!hit)
            return;
        if (critical && damage && (info->GetSchoolMask() & SPELL_SCHOOL_MASK_ARCANE))
            ExtraMoonfireTick(player, target);
        if (IsDruidSpell(info, EntanglingRootsFlag0))
            SilenceWithRoots(player, target);
        if (damage && IsDruidSpell(info, WrathFlag0))
            WrathOfCenarius(player);
    }

private:
    static void SpendMoonfury(Player* player, Unit* target, bool hit)
    {
        if (!player->HasAura(MoonfuryEmpowered))
            return;
        player->RemoveAurasDueToSpell(MoonfuryEmpowered);
        if (!hit)
            return;
        if (uint32 swarm = HighestKnownRank(player, InsectSwarm))
            player->CastSpell(target, swarm, true);
    }

    static void ExtraMoonfireTick(Player* player, Unit* target)
    {
        if (!player->HasAura(ImprovedMoonfireCapstone) || player->HasSpellCooldown(ImprovedMoonfireCapstone))
            return;
        AuraEffect* moonfire = OwnPeriodicDamage(target, player, MoonfireFlag0);
        AuraApplication* application = moonfire ? moonfire->GetBase()->GetApplicationOfTarget(target->GetGUID()) :
            nullptr;
        if (!application)
            return;
        player->AddSpellCooldown(ImprovedMoonfireCapstone, 0, MoonfireExtraTickCooldownMs);
        moonfire->PeriodicTick(application, player);
    }

    static void SilenceWithRoots(Player* player, Unit* target)
    {
        if (!player->HasAura(BramblesCapstone) || player->HasSpellCooldown(BramblesSilence))
            return;
        player->AddSpellCooldown(BramblesSilence, 0, BramblesSilenceCooldownMs);
        player->CastSpell(target, BramblesSilence, true);
    }

    static void WrathOfCenarius(Player* player)
    {
        if (!player->HasAura(WrathOfCenariusCapstone))
            return;
        player->ModifySpellCooldown(SolarBeam, -WrathOfCenariusStepMs);
        if (Aura* grace = player->GetAura(NaturesGraceBuff))
        {
            grace->SetMaxDuration(grace->GetMaxDuration() + WrathOfCenariusStepMs);
            grace->SetDuration(grace->GetDuration() + WrathOfCenariusStepMs);
        }
    }
};

class aura_area52_moonfury_dispel : public AuraScript
{
    PrepareAuraScript(aura_area52_moonfury_dispel);

    void Empower(DispelInfo*)
    {
        Player* caster = FreePickPlayer(GetCaster());
        if (caster && caster->HasAura(MoonfuryCapstone))
            caster->CastSpell(caster, MoonfuryEmpowered, true);
    }

    void Register() override
    {
        AfterDispel += AuraDispelFn(aura_area52_moonfury_dispel::Empower);
    }
};

class aura_area52_insect_swarm_capstones : public AuraScript
{
    PrepareAuraScript(aura_area52_insect_swarm_capstones);

    void Expose(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* caster = FreePickPlayer(GetCaster());
        if (!caster || !caster->HasAura(CelestialBaneCapstone))
            return;
        if (Aura* mastery = caster->AddAura(NatureMastery, GetTarget()))
        {
            mastery->SetMaxDuration(GetAura()->GetMaxDuration());
            mastery->SetDuration(GetAura()->GetDuration());
        }
    }

    void Cover(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(NatureMastery, GetCasterGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_insect_swarm_capstones::Expose, EFFECT_0,
            SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_insect_swarm_capstones::Cover, EFFECT_0,
            SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_natures_splendor : public AuraScript
{
    PrepareAuraScript(aura_area52_natures_splendor);

    void Grant(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* target = GetTarget();
        if (!FreePickPlayer(target))
            return;
        for (uint32 bonus : {SplendorDurations, SplendorFlourishAndInsectSwarm})
            if (!target->HasAura(bonus))
                target->AddAura(bonus, target);
    }

    void Revoke(AuraEffect const*, AuraEffectHandleModes)
    {
        for (uint32 bonus : {SplendorDurations, SplendorFlourishAndInsectSwarm})
            GetTarget()->RemoveAurasDueToSpell(bonus);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_natures_splendor::Grant, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_natures_splendor::Revoke, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_vengeful_soul : public AuraScript
{
    PrepareAuraScript(aura_area52_vengeful_soul);

    void Depart(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;
        Unit* target = GetTarget();
        if (uint32 mana = VengefulSoulMana(target->GetMaxPower(POWER_MANA), target->GetPower(POWER_MANA)))
            target->EnergizeBySpell(target, VengefulSoulDeparture, mana, POWER_MANA);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_vengeful_soul::Depart, EFFECT_0,
            SPELL_AURA_MOD_DAMAGE_PERCENT_DONE, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_natures_reach : public AuraScript
{
    PrepareAuraScript(aura_area52_natures_reach);

    bool CheckProc(ProcEventInfo& event)
    {
        Unit* source = event.GetActionTarget();
        return source && source != GetTarget() && FreePickPlayer(GetTarget()) &&
            OwnPeriodicDamage(source, GetTarget(), InsectSwarmFlag0);
    }

    void Spread(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* caster = GetTarget();
        Unit* source = event.GetActionTarget();
        AuraEffect* swarmEffect = source ? OwnPeriodicDamage(source, caster, InsectSwarmFlag0) : nullptr;
        if (!swarmEffect)
            return;
        Aura const* swarm = swarmEffect->GetBase();
        std::list<Unit*> candidates;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(source, caster, InsectSwarmSpreadRadius);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(source, candidates, check);
        Cell::VisitObjects(source, searcher, InsectSwarmSpreadRadius);
        std::size_t spread = 0;
        for (Unit* victim : candidates)
        {
            if (spread >= InsectSwarmSpreadMaxTargets)
                return;
            if (victim == source || victim->IsTotem() || !caster->IsValidAttackTarget(victim))
                continue;
            Aura* copy = victim->GetAura(swarm->GetId(), caster->GetGUID());
            if (!ShouldSpreadInsectSwarm(swarm->GetDuration(), copy ? copy->GetDuration() : 0, copy != nullptr))
                continue;
            if (!copy)
                copy = caster->AddAura(swarm->GetId(), victim);
            if (!copy)
                continue;
            copy->SetMaxDuration(swarm->GetMaxDuration());
            copy->SetDuration(swarm->GetDuration());
            ++spread;
        }
    }

    void Hold(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_natures_reach::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_natures_reach::Spread, EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL);
        OnEffectProc += AuraEffectProcFn(aura_area52_natures_reach::Hold, EFFECT_2, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};
}

void AddSC_Area52DruidBalanceCapstones()
{
    new area52_druid_balance_capstones();
    RegisterSpellScript(aura_area52_moonfury_dispel);
    RegisterSpellScript(aura_area52_insect_swarm_capstones);
    RegisterSpellScript(aura_area52_natures_splendor);
    RegisterSpellScript(aura_area52_vengeful_soul);
    RegisterSpellScript(aura_area52_natures_reach);
}
