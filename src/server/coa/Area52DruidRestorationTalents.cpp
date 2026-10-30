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

bool IsDruidSpell(SpellInfo const* info, uint32 flag0)
{
    return info && info->SpellFamilyName == SPELLFAMILY_DRUID && (info->SpellFamilyFlags[0] & flag0);
}

bool InTreeForm(Unit const* unit)
{
    return std::any_of(std::begin(TreeForms), std::end(TreeForms), [unit](uint32 form) { return unit->HasAura(form); });
}

class aura_area52_capstone_grant : public AuraScript
{
    PrepareAuraScript(aura_area52_capstone_grant);

    uint32 Granted() const
    {
        switch (GetId())
        {
            case NaturesFocusCapstone:
                return NaturesFocusGuard;
            case SubtletyCapstone:
                return CapstoneThreatReduction;
            default:
                return 0;
        }
    }

    void Grant(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* target = GetTarget();
        if (uint32 granted = Granted(); granted && FreePickPlayer(target) && !target->HasAura(granted))
            target->AddAura(granted, target);
    }

    void Revoke(AuraEffect const*, AuraEffectHandleModes)
    {
        if (uint32 granted = Granted())
            GetTarget()->RemoveAurasDueToSpell(granted);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_capstone_grant::Grant, EFFECT_0, SPELL_AURA_ANY,
            AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_capstone_grant::Revoke, EFFECT_0, SPELL_AURA_ANY,
            AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_natures_focus_guard : public AuraScript
{
    PrepareAuraScript(aura_area52_natures_focus_guard);

    bool CheckProc(ProcEventInfo&)
    {
        Spell const* cast = GetTarget()->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!cast)
            cast = GetTarget()->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        SpellInfo const* info = cast ? cast->GetSpellInfo() : nullptr;
        return info && (info->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE) &&
            (info->HasEffect(SPELL_EFFECT_HEAL) || info->HasAura(SPELL_AURA_PERIODIC_HEAL));
    }

    void Mend(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), NaturesFocusHeal, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_natures_focus_guard::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_natures_focus_guard::Mend, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_intensity_mana : public SpellScript
{
    PrepareSpellScript(spell_area52_intensity_mana);

    void BaseMana(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target || !FreePickPlayer(target))
            return;
        PreventHitDefaultEffect(effIndex);
        if (uint32 mana = PctOf(target->GetCreateMana(), uint32(std::max(0, GetEffectValue()))))
            GetCaster()->EnergizeBySpell(target, GetSpellInfo()->Id, mana, POWER_MANA);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_intensity_mana::BaseMana, EFFECT_0, SPELL_EFFECT_ENERGIZE);
    }
};

class aura_area52_improved_tree_of_life : public AuraScript
{
    PrepareAuraScript(aura_area52_improved_tree_of_life);

    void OnlyInTree(AuraEffect const*, int32& amount, bool& canBeRecalculated)
    {
        canBeRecalculated = true;
        if (!GetUnitOwner() || !InTreeForm(GetUnitOwner()))
            amount = 0;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_improved_tree_of_life::OnlyInTree, EFFECT_0,
            SPELL_AURA_MOD_BASE_RESISTANCE_PCT);
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_improved_tree_of_life::OnlyInTree, EFFECT_1,
            SPELL_AURA_MOD_SPELL_HEALING_OF_STAT_PERCENT);
    }
};

class aura_area52_tree_form_talents : public AuraScript
{
    PrepareAuraScript(aura_area52_tree_form_talents);

    void Refresh(AuraEffect const*, AuraEffectHandleModes)
    {
        if (Aura* improved = GetTarget()->GetAuraOfRankedSpell(ImprovedTreeOfLife))
            improved->RecalculateAmountOfEffects();
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_tree_form_talents::Refresh, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_tree_form_talents::Refresh, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};

class area52_druid_restoration_capstones : public AllSpellScript
{
public:
    area52_druid_restoration_capstones() :
        AllSpellScript("area52_druid_restoration_capstones", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo, uint32, uint32 healing, bool) override
    {
        Player* player = FreePickPlayer(spell->GetCaster());
        if (!player || !target || missInfo != SPELL_MISS_NONE || !healing ||
            !IsDruidSpell(spell->GetSpellInfo(), HealingTouchFlag0) || !player->HasAura(RevitalizeCapstone) ||
            !target->IsFullHealth() || spell->GetSpellInfo()->PowerType != POWER_MANA)
            return;
        if (uint32 mana = PctOf(uint32(std::max(0, spell->GetPowerCost())), RevitalizeRefundPct))
            player->EnergizeBySpell(player, RevitalizeCapstone, mana, POWER_MANA);
    }
};

class area52_living_spirit : public UnitScript
{
public:
    area52_living_spirit() : UnitScript("area52_living_spirit", true, {UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK}) { }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* caster, uint32& amount, SpellInfo const* info) override
    {
        Player* player = FreePickPlayer(caster);
        if (player && target && IsDruidSpell(info, RejuvenationFlag0) && info->IsPositive() &&
            player->HasAura(LivingSpiritCapstone) && LivingSpiritApplies(target->GetHealthPct()))
            amount += PctOf(amount, LivingSpiritBonusPct);
    }
};
}

void AddSC_Area52DruidRestorationTalents()
{
    RegisterSpellScript(aura_area52_capstone_grant);
    RegisterSpellScript(aura_area52_natures_focus_guard);
    RegisterSpellScript(spell_area52_intensity_mana);
    RegisterSpellScript(aura_area52_improved_tree_of_life);
    RegisterSpellScript(aura_area52_tree_form_talents);
    new area52_druid_restoration_capstones();
    new area52_living_spirit();
}
