/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionWhiteWalkerSupportRules.h"
#include "Config.h"
#include "AllItemScript.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace
{
bool Area52Enabled = false;
std::unordered_map<ObjectGuid::LowType, std::unordered_set<uint32>> WhiteWalkerGranted;
std::unordered_set<ObjectGuid::LowType> AbsoluteZeroGranted;
std::unordered_map<ObjectGuid::LowType, uint32> FreezingBloodLastCooldown;

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

void RefreshAbsoluteZero(Player* player)
{
    using namespace AscensionWhiteWalkerSupport;
    bool const active = Area52Enabled && player->HasAura(AbsoluteZero) && player->HasActiveSpell(FrostNova);
    bool const knowsReplacement = player->HasActiveSpell(AbsoluteZeroReplacement);
    auto const guid = player->GetGUID().GetCounter();
    if (active && !knowsReplacement)
    {
        player->learnSpell(AbsoluteZeroReplacement, true);
        AbsoluteZeroGranted.insert(guid);
    }
    if (!active)
    {
        player->SetTemporarySpellReplacement(FrostNova, 0);
        if (AbsoluteZeroGranted.erase(guid) && player->GetSpellMap().find(AbsoluteZeroReplacement) != player->GetSpellMap().end())
            player->removeSpell(AbsoluteZeroReplacement, SPEC_MASK_ALL, true);
        return;
    }
    player->SetTemporarySpellReplacement(FrostNova, AbsoluteZeroReplacement);
}

class white_walker_player : public PlayerScript
{
public:
    white_walker_player() : PlayerScript("white_walker_player", { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_LOGOUT }) { }

    void OnPlayerLogin(Player* player) override
    {
        RefreshWhiteWalker(player);
        RefreshAbsoluteZero(player);
    }

    void OnPlayerUpdate(Player* player, uint32) override
    {
        RefreshWhiteWalker(player);
        RefreshAbsoluteZero(player);
    }

    void OnPlayerLogout(Player* player) override
    {
        WhiteWalkerGranted.erase(player->GetGUID().GetCounter());
        AbsoluteZeroGranted.erase(player->GetGUID().GetCounter());
        FreezingBloodLastCooldown.erase(player->GetGUID().GetCounter());
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
        if (info->Id == AscensionWhiteWalkerSupport::FreezingBlood)
        {
            info->ProcFlags = 16;
            info->ProcChance = 101;
            return;
        }
        if (info->Id == AscensionWhiteWalkerSupport::FightersSinisterStrike)
        {
            info->ProcFlags = 16;
            info->ProcChance = 101;
            return;
        }
        if (info->Id == AscensionWhiteWalkerSupport::TidesOfBattle)
        {
            info->ProcFlags = 16;
            info->ProcChance = 101;
            return;
        }
        if (info->Id == 20101 || info->Id == 20102 || info->Id == 20103)
        {
            info->ProcFlags = PROC_FLAG_KILL;
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

class item_area52_frostbrand : public AllItemScript
{
public:
    item_area52_frostbrand() : AllItemScript("item_area52_frostbrand") { }

    bool OnItemCombatSpell(Player* player, Unit* victim, SpellInfo const* spellInfo, Item* item) override
    {
        if (!Area52Enabled || !player || !victim || !spellInfo)
            return true;
        using namespace AscensionWhiteWalkerSupport;
        auto const it = std::find(FrostbrandDamageHelpers.begin(), FrostbrandDamageHelpers.end(), spellInfo->Id);
        if (it == FrostbrandDamageHelpers.end())
            return true;
        std::size_t const rank = static_cast<std::size_t>(std::distance(FrostbrandDamageHelpers.begin(), it));
        player->CastSpell(victim, FrostbrandSlowHelpers[rank], TRIGGERED_FULL_MASK, item);
        return true;
    }
};

class player_area52_frostbrand : public PlayerScript
{
public:
    player_area52_frostbrand() : PlayerScript("player_area52_frostbrand") { }

    static bool HasItemHelper(Player* player, uint32 helper, ObjectGuid itemGuid)
    {
        auto const range = player->GetAppliedAuras().equal_range(helper);
        for (auto itr = range.first; itr != range.second; ++itr)
            if (itr->second->GetBase()->GetCastItemGUID() == itemGuid)
                return true;
        return false;
    }

    bool OnPlayerCanApplyEnchantment(Player* player, Item* item, EnchantmentSlot slot, bool apply, bool, bool) override
    {
        if (!Area52Enabled || !player || !item || slot != TEMP_ENCHANTMENT_SLOT)
            return true;

        using namespace AscensionWhiteWalkerSupport;
        uint32 const enchantment = item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
        auto const enchantmentIt = std::find(FrostbrandEnchantments.begin(), FrostbrandEnchantments.end(), enchantment);
        if (enchantmentIt == FrostbrandEnchantments.end())
            return true;

        std::size_t const rank = static_cast<std::size_t>(std::distance(FrostbrandEnchantments.begin(), enchantmentIt));
        uint32 const helper = FrostbrandDamageHelpers[rank];
        if (apply)
        {
            for (uint32 otherHelper : FrostbrandDamageHelpers)
                if (otherHelper != helper)
                    player->RemoveAurasDueToItemSpell(otherHelper, item->GetGUID());
            if (!HasItemHelper(player, helper, item->GetGUID()))
                player->CastSpell(player, helper, TRIGGERED_FULL_MASK, item);
        }
        else
            player->RemoveAurasDueToItemSpell(helper, item->GetGUID());
        return true;
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
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::WintersFury, TRIGGERED_FULL_MASK);
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::WintersVitality, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_slice_and_ice::CheckProc);
        OnProc += AuraProcFn(aura_area52_slice_and_ice::Proc);
    }
};

class aura_area52_freezing_blood : public AuraScript
{
    PrepareAuraScript(aura_area52_freezing_blood);

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!Area52Enabled || !player || !event.GetSpellInfo())
            return false;
        if (event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::ColdBlood)
            return player->HasActiveSpell(AscensionWhiteWalkerSupport::Frostbolt);
        auto const* damage = event.GetDamageInfo();
        return damage && damage->GetDamage() && (event.GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) &&
            !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC);
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;
        if (event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::ColdBlood)
        {
            player->CastSpell(player, AscensionWhiteWalkerSupport::Frostbolt, TRIGGERED_FULL_MASK);
            return;
        }
        uint32 const now = getMSTime();
        uint32& last = FreezingBloodLastCooldown[player->GetGUID().GetCounter()];
        if (last && getMSTimeDiff(last, now) < 1000)
            return;
        last = now;
        player->ModifySpellCooldown(AscensionWhiteWalkerSupport::ColdBlood, -10000);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_freezing_blood::CheckProc);
        OnProc += AuraProcFn(aura_area52_freezing_blood::Proc);
    }
};

class aura_area52_fighters_sinister_strike : public AuraScript
{
    PrepareAuraScript(aura_area52_fighters_sinister_strike);

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52Enabled && event.GetSpellInfo() && event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::SinisterStrike &&
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::FightersPower, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_fighters_sinister_strike::CheckProc);
        OnProc += AuraProcFn(aura_area52_fighters_sinister_strike::Proc);
    }
};

class aura_area52_tides_of_battle : public AuraScript
{
    PrepareAuraScript(aura_area52_tides_of_battle);

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52Enabled && event.GetSpellInfo() && event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::SinisterStrike &&
            GetTarget()->ToPlayer() && event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
    }

    void Proc(ProcEventInfo&)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;
        bool const aboveThreshold = player->GetMaxPower(POWER_ENERGY) > 0 &&
            player->GetPower(POWER_ENERGY) * 100 >= player->GetMaxPower(POWER_ENERGY) * 90;
        player->RemoveAurasDueToSpell(aboveThreshold ? AscensionWhiteWalkerSupport::Flow : AscensionWhiteWalkerSupport::Ebb);
        player->CastSpell(player, aboveThreshold ? AscensionWhiteWalkerSupport::Ebb : AscensionWhiteWalkerSupport::Flow,
            TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_tides_of_battle::CheckProc);
        OnProc += AuraProcFn(aura_area52_tides_of_battle::Proc);
    }
};
}

void AddSC_AscensionWhiteWalkerSupport()
{
    new white_walker_support_metadata();
    new white_walker_player();
    new player_area52_frostbrand();
    RegisterSpellScript(spell_area52_honed_ice);
    new item_area52_frostbrand();
    RegisterSpellScript(aura_area52_white_walker);
    RegisterSpellScript(aura_area52_slice_and_ice);
    RegisterSpellScript(aura_area52_freezing_blood);
    RegisterSpellScript(aura_area52_fighters_sinister_strike);
    RegisterSpellScript(aura_area52_tides_of_battle);
}
