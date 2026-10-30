/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AllSpellScript.h"
#include "Area52DruidTalentRules.h"
#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <algorithm>

namespace
{
using namespace Area52DruidTalents;

Player* FreePickPlayer(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return AscensionFreePick::Applies(player) ? player : nullptr;
}

bool IsDruidSpell(SpellInfo const* info, uint32 flag0, uint32 flag1 = 0)
{
    return info && info->SpellFamilyName == SPELLFAMILY_DRUID &&
        ((info->SpellFamilyFlags[0] & flag0) || (info->SpellFamilyFlags[1] & flag1));
}

void ShortenChainCooldown(Player* player, uint32 firstRank, int32 ms)
{
    for (uint32 rank = firstRank; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (player->HasSpellCooldown(rank))
            player->ModifySpellCooldown(rank, -ms);
}

class aura_area52_savage_defense_talent : public AuraScript
{
    PrepareAuraScript(aura_area52_savage_defense_talent);

    bool CheckProc(ProcEventInfo& event)
    {
        if (!(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC))
            return true;
        Player* player = FreePickPlayer(GetTarget());
        return player && player->HasAura(PrimalGoreCapstone) && IsDruidSpell(event.GetSpellInfo(), 0, LacerateFlag1);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_savage_defense_talent::CheckProc);
    }
};

class aura_area52_feral_aggression : public AuraScript
{
    PrepareAuraScript(aura_area52_feral_aggression);

    void ShareOfLevel(AuraEffect const*, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = true;
        amount = FeralAggressionAttackPower(GetUnitOwner() ? GetUnitOwner()->GetLevel() : 0, amount);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_feral_aggression::ShareOfLevel, EFFECT_1,
            SPELL_AURA_MOD_ATTACK_POWER);
    }
};

class area52_feral_aggression_level : public PlayerScript
{
public:
    area52_feral_aggression_level() : PlayerScript("area52_feral_aggression_level", {PLAYERHOOK_ON_LEVEL_CHANGED}) { }

    void OnPlayerLevelChanged(Player* player, uint8) override
    {
        if (Aura* aggression = player->GetAuraOfRankedSpell(FeralAggression))
            aggression->RecalculateAmountOfEffects();
    }
};

class area52_feral_damage : public UnitScript
{
public:
    area52_feral_damage() : UnitScript("area52_feral_damage", true, {UNITHOOK_MODIFY_MELEE_DAMAGE,
        UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN, UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK}) { }

    static int32 DonePct(Unit* attacker)
    {
        Player* player = FreePickPlayer(attacker);
        if (!player)
            return 0;
        AuraEffect const* jungle = player->GetAuraEffectOfRankedSpell(KingOfTheJungle, EFFECT_2);
        return FeralDamagePct(player->GetAuraOfRankedSpell(TigersFury) != nullptr,
            player->GetAuraOfRankedSpell(FeralInstinct) != nullptr, player->GetHealthPct() > 75.0f,
            jungle ? jungle->GetAmount() : 0);
    }

    static int32 TakenPct(Unit* target)
    {
        Player* player = FreePickPlayer(target);
        return player && player->HasAura(Enrage) && player->GetAuraOfRankedSpell(FeralInstinct) ?
            -FeralInstinctPct : 0;
    }

    static int32 Adjusted(int32 damage, Unit* target, Unit* attacker)
    {
        int32 const pct = DonePct(attacker) + TakenPct(target);
        return pct ? std::max(0, damage + damage * pct / 100) : damage;
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        damage = uint32(Adjusted(int32(damage), target, attacker));
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const*) override
    {
        if (damage > 0)
            damage = Adjusted(damage, target, attacker);
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* info) override
    {
        if (info && !info->IsPositive())
            damage = uint32(Adjusted(int32(damage), target, attacker));
    }
};

class area52_druid_feral_capstones : public AllSpellScript
{
public:
    area52_druid_feral_capstones() :
        AllSpellScript("area52_druid_feral_capstones", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo, uint32, uint32, bool) override
    {
        Player* player = FreePickPlayer(spell->GetCaster());
        SpellInfo const* info = spell->GetSpellInfo();
        if (!player || !target || target == player || !info || missInfo != SPELL_MISS_NONE)
            return;
        if (IsDruidSpell(info, BashFlag0) || info->Id == FeralChargeBear)
            BrutalImpact(player, target);
        if (IsDruidSpell(info, 0, MangleFlag1))
            ImprovedMangle(player);
    }

private:
    static void BrutalImpact(Player* player, Unit* target)
    {
        SpellInfo const* strike = sSpellMgr->GetSpellInfo(BrutalImpactStrike);
        if (!strike || !player->HasAura(BrutalImpactCapstone))
            return;
        int32 damage = int32(player->GetTotalAttackPowerValue(BASE_ATTACK)) * strike->Effects[EFFECT_1].CalcValue() /
            100;
        if (damage > 0)
            player->CastCustomSpell(target, BrutalImpactStrike, &damage, nullptr, nullptr, true);
    }

    static void ImprovedMangle(Player* player)
    {
        if (!player->HasAura(ImprovedMangleCapstone) || player->HasSpellCooldown(ImprovedMangleCapstone))
            return;
        player->AddSpellCooldown(ImprovedMangleCapstone, 0, ImprovedMangleCooldownMs);
        ShortenChainCooldown(player, Enrage, ImprovedMangleCooldownMs);
        ShortenChainCooldown(player, TigersFury, ImprovedMangleCooldownMs);
    }
};

class aura_area52_king_of_the_jungle : public AuraScript
{
    PrepareAuraScript(aura_area52_king_of_the_jungle);

    void Quicken(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = FreePickPlayer(GetTarget());
        if (!player || _shift)
            return;
        _shift = new SpellModifier(GetAura());
        _shift->op = SPELLMOD_GLOBAL_COOLDOWN;
        _shift->type = SPELLMOD_FLAT;
        _shift->value = KingOfTheJungleGcdMs;
        _shift->mask = flag96(CatFormFlag0 | BearFormFlag0, 0, 0);
        _shift->spellId = GetId();
        player->AddSpellMod(_shift, true);
    }

    void Restore(AuraEffect const*, AuraEffectHandleModes)
    {
        if (!_shift)
            return;
        if (Player* player = GetTarget()->ToPlayer())
            player->AddSpellMod(_shift, false);
        delete _shift;
        _shift = nullptr;
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_king_of_the_jungle::Quicken, EFFECT_2,
            SPELL_AURA_OVERRIDE_CLASS_SCRIPTS, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_king_of_the_jungle::Restore, EFFECT_2,
            SPELL_AURA_OVERRIDE_CLASS_SCRIPTS, AURA_EFFECT_HANDLE_REAL);
    }

    SpellModifier* _shift = nullptr;
};

class aura_area52_elder_hide : public AuraScript
{
    PrepareAuraScript(aura_area52_elder_hide);

    void Grant(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* target = GetTarget();
        if (FreePickPlayer(target) && !target->HasAura(IronfurHealing))
            target->AddAura(IronfurHealing, target);
    }

    void Revoke(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(IronfurHealing);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_elder_hide::Grant, EFFECT_1, SPELL_AURA_MOD_PERCENT_STAT,
            AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_elder_hide::Revoke, EFFECT_1, SPELL_AURA_MOD_PERCENT_STAT,
            AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_dreamstate_mana : public SpellScript
{
    PrepareSpellScript(spell_area52_dreamstate_mana);

    void MissingMana(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target || !FreePickPlayer(target))
            return;
        PreventHitDefaultEffect(effIndex);
        if (uint32 mana = MissingManaShare(target->GetMaxPower(POWER_MANA), target->GetPower(POWER_MANA),
                uint32(std::max(0, GetEffectValue()))))
            GetCaster()->EnergizeBySpell(target, GetSpellInfo()->Id, mana, POWER_MANA);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_dreamstate_mana::MissingMana, EFFECT_0,
            SPELL_EFFECT_ENERGIZE_PCT);
    }
};

class aura_area52_earth_and_moon : public AuraScript
{
    PrepareAuraScript(aura_area52_earth_and_moon);

    void Wound(AuraEffect const*, AuraEffectHandleModes)
    {
        if (Unit* caster = GetCaster(); FreePickPlayer(caster))
            caster->AddAura(EarthAndMoonHealingReduction, GetTarget());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_earth_and_moon::Wound, EFFECT_1,
            SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};
}

void AddSC_Area52DruidFeralTalents()
{
    RegisterSpellScript(aura_area52_savage_defense_talent);
    RegisterSpellScript(aura_area52_feral_aggression);
    new area52_feral_aggression_level();
    new area52_feral_damage();
    new area52_druid_feral_capstones();
    RegisterSpellScript(aura_area52_king_of_the_jungle);
    RegisterSpellScript(aura_area52_elder_hide);
    RegisterSpellScript(spell_area52_dreamstate_mana);
    RegisterSpellScript(aura_area52_earth_and_moon);
}
