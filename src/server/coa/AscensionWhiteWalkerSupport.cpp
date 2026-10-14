/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionWhiteWalkerSupportRules.h"
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"

namespace
{
bool Area52Enabled = false;

class white_walker_support_metadata : public GlobalScript
{
public:
    white_walker_support_metadata() : GlobalScript("white_walker_support_metadata")
    {
        Area52Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live" &&
            !(sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400));
    }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Area52Enabled)
            return;

        if (info->Id == AscensionWhiteWalkerSupport::HonedIceBuff)
        {
            info->StackAmount = 1;
            return;
        }
        if (info->Id != AscensionWhiteWalkerSupport::AthleticIcing)
            return;

        info->StackAmount = 1;
        auto& cost = info->Effects[EFFECT_0];
        cost.Effect = SPELL_EFFECT_APPLY_AURA;
        cost.ApplyAuraName = SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT;
        cost.BasePoints = AscensionWhiteWalkerSupport::AthleticCostPercent - 1;
        cost.DieSides = 1;
        cost.MiscValue = AscensionWhiteWalkerSupport::AthleticSchools;
        cost.TriggerSpell = 0;
        cost.TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
        cost.TargetB = SpellImplicitTargetInfo();
        info->Effects[EFFECT_1].Effect = 0;
        info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_NONE;
        info->Effects[EFFECT_1].TriggerSpell = 0;
    }
};

class spell_area52_honed_ice : public AuraScript
{
    PrepareAuraScript(spell_area52_honed_ice);

    bool Load() override
    {
        return Area52Enabled && GetUnitOwner()->IsPlayer() && GetUnitOwner()->getClass() == 10;
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({AscensionWhiteWalkerSupport::HonedIceBuff});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        auto const* spell = event.GetSpellInfo();
        auto const* damage = event.GetDamageInfo();
        return spell && damage && AscensionWhiteWalkerSupport::HonedIceSource(spell->SpellFamilyName,
            spell->SpellFamilyFlags[0], event.GetSchoolMask(), event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC,
            damage->GetDamage(), event.GetActor() == GetTarget());
    }

    void Proc(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        auto const* buff = sSpellMgr->GetSpellInfo(AscensionWhiteWalkerSupport::HonedIceBuff);
        int32 const amount = AscensionWhiteWalkerSupport::HonedIceCriticalBonus(
            buff->Effects[EFFECT_0].CalcValue(GetTarget()), GetStackAmount());
        GetTarget()->CastCustomSpell(AscensionWhiteWalkerSupport::HonedIceBuff,
            SPELLVALUE_BASE_POINT0, amount, GetTarget(), true, nullptr, effect);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(AscensionWhiteWalkerSupport::HonedIceBuff, GetTarget()->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_honed_ice::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_honed_ice::Proc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_honed_ice::Remove,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};
}

void AddSC_AscensionWhiteWalkerSupport()
{
    new white_walker_support_metadata();
    RegisterSpellScript(spell_area52_honed_ice);
}
