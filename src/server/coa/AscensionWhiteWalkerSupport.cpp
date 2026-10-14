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
#include <unordered_map>
#include <unordered_set>

namespace
{
bool Area52Enabled = false;
std::unordered_map<ObjectGuid::LowType, std::unordered_set<uint32>> WhiteWalkerGranted;

bool IsWhiteWalkerManagedSpell(uint32 spellId)
{
    using namespace AscensionWhiteWalkerSupport;
    if (spellId == SliceAndIce || spellId == IceSpear)
        return true;
    for (uint32 rank : WindsOfWinterRanks)
        if (spellId == rank)
            return true;
    return false;
}

void RemoveGranted(Player* player, std::unordered_set<uint32>& granted, uint32 keep = 0)
{
    for (auto it = granted.begin(); it != granted.end();)
    {
        uint32 const spellId = *it;
        if (spellId != keep && IsWhiteWalkerManagedSpell(spellId))
        {
            if (player->GetSpellMap().find(spellId) != player->GetSpellMap().end())
                player->removeSpell(spellId, SPEC_MASK_ALL, true);
            it = granted.erase(it);
        }
        else
            ++it;
    }
}

void RefreshWhiteWalker(Player* player)
{
    using namespace AscensionWhiteWalkerSupport;
    bool const active = Area52Enabled && player->HasAura(WhiteWalker) && player->IsTwoHandUsed();
    auto& granted = WhiteWalkerGranted[player->GetGUID().GetCounter()];
    uint32 windsReplacement = 0;
    for (auto rank = WindsOfWinterRanks.rbegin(); rank != WindsOfWinterRanks.rend(); ++rank)
        if (player->HasActiveSpell(*rank))
        {
            windsReplacement = *rank;
            break;
        }
    if (!active)
        RemoveGranted(player, granted);

    for (auto const& [root, replacement] : { std::pair<uint32, uint32>{ SliceAndDice, SliceAndIce },
             { WindsOfWinter, windsReplacement } })
    {
        if (active)
            for (uint32 oldRank : WindsOfWinterRanks)
                if (oldRank != replacement && granted.contains(oldRank))
                {
                    if (player->GetSpellMap().find(oldRank) != player->GetSpellMap().end())
                        player->removeSpell(oldRank, SPEC_MASK_ALL, true);
                    granted.erase(oldRank);
                }
        if (active && replacement && player->HasActiveSpell(root) && !player->HasActiveSpell(replacement))
        {
            player->learnSpell(replacement, true);
            granted.insert(replacement);
        }
        player->SetTemporarySpellReplacement(root, active ? replacement : 0);
    }
    uint32 iceSpear = 0;
    for (auto rank = IceSpearRanks.rbegin(); rank != IceSpearRanks.rend(); ++rank)
        if (player->HasActiveSpell(*rank))
        {
            iceSpear = *rank;
            break;
        }
    if (active && player->HasActiveSpell(IceLance) && iceSpear == 0)
    {
        player->learnSpell(IceSpear, true);
        granted.insert(IceSpear);
    }
    if (active && iceSpear != 0 && granted.contains(IceSpear))
    {
        if (iceSpear != IceSpear && player->GetSpellMap().find(IceSpear) != player->GetSpellMap().end())
            player->removeSpell(IceSpear, SPEC_MASK_ALL, true);
        granted.erase(IceSpear);
    }
}

class white_walker_player : public PlayerScript
{
public:
    white_walker_player() : PlayerScript("white_walker_player", { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_LOGOUT }) { }

    void OnPlayerLogin(Player* player) override
    {
        RefreshWhiteWalker(player);
    }

    void OnPlayerUpdate(Player* player, uint32) override
    {
        RefreshWhiteWalker(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        WhiteWalkerGranted.erase(player->GetGUID().GetCounter());
    }
};

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
        if (info->Id == AscensionWhiteWalkerSupport::SliceAndIce)
        {
            info->ProcFlags = 16;
            info->ProcChance = 101;
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

class aura_area52_white_walker : public AuraScript
{
    PrepareAuraScript(aura_area52_white_walker);

    bool CheckProc(ProcEventInfo& event)
    {
        if (!Area52Enabled || !event.GetSpellInfo() || !(event.GetHitMask() & PROC_HIT_CRITICAL))
            return false;
        uint32 const spellId = event.GetSpellInfo()->Id;
        return spellId == 1752 || spellId == 2098;
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* target = event.GetActionTarget();
        if (target && target->IsAlive())
            GetTarget()->CastSpell(target, AscensionWhiteWalkerSupport::IceLance, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_white_walker::CheckProc);
        OnProc += AuraProcFn(aura_area52_white_walker::Proc);
    }
};

class aura_area52_slice_and_ice : public AuraScript
{
    PrepareAuraScript(aura_area52_slice_and_ice);

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52Enabled && event.GetSpellInfo() && event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::IceLance &&
            (event.GetHitMask() & PROC_HIT_CRITICAL);
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), 978525, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_slice_and_ice::CheckProc);
        OnProc += AuraProcFn(aura_area52_slice_and_ice::Proc);
    }
};
}

void AddSC_AscensionWhiteWalkerSupport()
{
    new white_walker_support_metadata();
    new white_walker_player();
    RegisterSpellScript(spell_area52_honed_ice);
    RegisterSpellScript(aura_area52_white_walker);
    RegisterSpellScript(aura_area52_slice_and_ice);
}
