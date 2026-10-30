/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AllSpellScript.h"
#include "Area52HunterTalentRules.h"
#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"

namespace
{
using namespace Area52HunterTalents;

Player* FreePickPlayer(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return AscensionFreePick::Applies(player) ? player : nullptr;
}

bool IsHunterSpell(SpellInfo const* info, uint32 flag1, uint32 flag2)
{
    return info && info->SpellFamilyName == SPELLFAMILY_HUNTER &&
        ((info->SpellFamilyFlags[1] & flag1) || (info->SpellFamilyFlags[2] & flag2));
}

uint32 HighestKnownRank(Player* player, uint32 firstRank)
{
    uint32 known = 0;
    for (uint32 rank = firstRank; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (player->HasSpell(rank))
            known = rank;
    return known;
}

class aura_area52_hunter_rank_grant : public AuraScript
{
    PrepareAuraScript(aura_area52_hunter_rank_grant);

    void Grant(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* target = GetTarget();
        if (uint32 helper = RankGrantHelper(GetId()); helper && FreePickPlayer(target) && !target->HasAura(helper))
            target->AddAura(helper, target);
    }

    void Revoke(AuraEffect const*, AuraEffectHandleModes)
    {
        if (uint32 helper = RankGrantHelper(GetId()))
            GetTarget()->RemoveAurasDueToSpell(helper);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_hunter_rank_grant::Grant, EFFECT_0, SPELL_AURA_ANY,
            AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_hunter_rank_grant::Revoke, EFFECT_0, SPELL_AURA_ANY,
            AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_mortal_shots : public AuraScript
{
    PrepareAuraScript(aura_area52_mortal_shots);

    void Lengthen(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = FreePickPlayer(GetTarget());
        if (!player || _duration)
            return;
        _duration = new SpellModifier(GetAura());
        _duration->op = SPELLMOD_DURATION;
        _duration->type = SPELLMOD_FLAT;
        _duration->value = MortalShotsAimedShotDurationMs(GetId());
        _duration->mask = flag96(AimedShotFlag0, 0, 0);
        _duration->spellId = GetId();
        player->AddSpellMod(_duration, true);
    }

    void Restore(AuraEffect const*, AuraEffectHandleModes)
    {
        if (!_duration)
            return;
        if (Player* player = GetTarget()->ToPlayer())
            player->AddSpellMod(_duration, false);
        delete _duration;
        _duration = nullptr;
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_mortal_shots::Lengthen, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_mortal_shots::Restore, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }

    SpellModifier* _duration = nullptr;
};

class aura_area52_improved_trueshot_aura : public AuraScript
{
    PrepareAuraScript(aura_area52_improved_trueshot_aura);

    void Follow(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        Unit* target = GetTarget();
        uint32 const buff = aurEff->GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell;
        if (!FreePickPlayer(target) || !target->HasAura(TrueshotAura, target->GetGUID()))
        {
            target->RemoveAurasDueToSpell(buff);
            return;
        }
        if (target->HasAura(buff))
            return;
        if (Aura* added = target->AddAura(buff, target))
        {
            added->SetMaxDuration(-1);
            added->SetDuration(-1);
        }
    }

    void Remove(AuraEffect const* aurEff, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(aurEff->GetSpellInfo()->Effects[aurEff->GetEffIndex()].TriggerSpell);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_improved_trueshot_aura::Follow, EFFECT_0,
            SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_improved_trueshot_aura::Remove, EFFECT_0,
            SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_improved_steady_shot : public AuraScript
{
    PrepareAuraScript(aura_area52_improved_steady_shot);

    bool CheckProc(ProcEventInfo&)
    {
        Unit* target = GetTarget();
        return !(FreePickPlayer(target) &&
            SteadyShotCapstoneApplies(GetId() == ImprovedSteadyShotCapstone, target->isMoving()));
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_improved_steady_shot::CheckProc);
    }
};

class aura_area52_capstone_steady_shot_buff : public AuraScript
{
    PrepareAuraScript(aura_area52_capstone_steady_shot_buff);

    void Spent(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(CapstoneArmorPierce);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_capstone_steady_shot_buff::Spent, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_serpent_sting_dispel : public AuraScript
{
    PrepareAuraScript(aura_area52_serpent_sting_dispel);

    void Kiss(DispelInfo*)
    {
        Player* caster = FreePickPlayer(GetCaster());
        Unit* target = GetUnitOwner();
        if (caster && target && target->IsAlive() && caster->HasAura(ImprovedStingsCapstone))
            caster->CastSpell(target, SerpentsKiss, true);
    }

    void Register() override
    {
        AfterDispel += AuraDispelFn(aura_area52_serpent_sting_dispel::Kiss);
    }
};

class area52_hunter_marksmanship_capstones : public AllSpellScript
{
public:
    area52_hunter_marksmanship_capstones() :
        AllSpellScript("area52_hunter_marksmanship_capstones", {ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo, uint32, uint32, bool) override
    {
        Player* player = FreePickPlayer(spell->GetCaster());
        SpellInfo const* info = spell->GetSpellInfo();
        if (!player || !target || target == player || missInfo != SPELL_MISS_NONE ||
            info->DmgClass != SPELL_DAMAGE_CLASS_RANGED)
            return;
        if (IsHunterSpell(info, SteadyShotFlag1, 0))
            SteadyShotCapstone(player);
        if (IsHunterSpell(info, 0, ChimeraShotFlag2))
            SerpentsKissStrike(player, target);
    }

private:
    static void SteadyShotCapstone(Player* player)
    {
        if (!SteadyShotCapstoneApplies(player->HasAura(ImprovedSteadyShotCapstone), player->isMoving()))
            return;
        player->CastSpell(player, CapstoneSteadyShotBuff, true);
        player->CastSpell(player, CapstoneArmorPierce, true);
    }

    static void SerpentsKissStrike(Player* player, Unit* target)
    {
        if (!target->HasAura(SerpentsKiss, player->GetGUID()))
            return;
        target->RemoveAurasDueToSpell(SerpentsKiss, player->GetGUID());
        player->CastSpell(target, SerpentsKissDamage, true);
        if (uint32 sting = HighestKnownRank(player, SerpentSting))
            player->CastSpell(target, sting, true);
    }
};
}

void AddSC_Area52HunterMarksmanshipTalents()
{
    RegisterSpellScript(aura_area52_hunter_rank_grant);
    RegisterSpellScript(aura_area52_mortal_shots);
    RegisterSpellScript(aura_area52_improved_trueshot_aura);
    RegisterSpellScript(aura_area52_improved_steady_shot);
    RegisterSpellScript(aura_area52_capstone_steady_shot_buff);
    RegisterSpellScript(aura_area52_serpent_sting_dispel);
    new area52_hunter_marksmanship_capstones();
}
