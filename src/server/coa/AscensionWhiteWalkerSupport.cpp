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

void RefreshWhiteWalker(Player* player)
{
    using namespace AscensionWhiteWalkerSupport;
    bool const active = Area52Enabled && player->HasAura(WhiteWalker) && player->IsTwoHandUsed();
    auto& granted = WhiteWalkerGranted[player->GetGUID().GetCounter()];
    std::unordered_set<uint32> desired;
    auto replace = [&](uint32 root, uint32 replacement)
    {
        bool const eligible = active && player->HasActiveSpell(root);
        if (eligible)
        {
            desired.insert(replacement);
            if (!player->HasActiveSpell(replacement))
            {
                player->learnSpell(replacement, true);
                granted.insert(replacement);
            }
        }
        player->SetTemporarySpellReplacement(root, eligible ? replacement : 0);
    };
    for (uint32 root : SliceAndDiceRanks)
        replace(root, SliceAndIce);
    for (std::size_t rank = 0; rank < ConeOfColdRanks.size(); ++rank)
        replace(ConeOfColdRanks[rank], WindsOfWinterRanks[rank]);
    uint32 iceSpear = 0;
    for (auto rank = IceSpearRanks.rbegin(); rank != IceSpearRanks.rend(); ++rank)
        if (player->HasActiveSpell(*rank))
        {
            iceSpear = *rank;
            break;
        }
    bool const iceLanceKnown = std::any_of(IceLanceRanks.begin(), IceLanceRanks.end(),
        [player](uint32 rank) { return player->HasActiveSpell(rank); });
    if (active && iceLanceKnown && iceSpear == 0)
    {
        player->learnSpell(IceSpear, true);
        granted.insert(IceSpear);
    }
    if (active && iceLanceKnown && (!iceSpear || iceSpear == IceSpear))
        desired.insert(IceSpear);
    for (auto it = granted.begin(); it != granted.end();)
    {
        if (!desired.contains(*it))
        {
            if (player->GetSpellMap().find(*it) != player->GetSpellMap().end())
                player->removeSpell(*it, SPEC_MASK_ALL, true);
            it = granted.erase(it);
        }
        else
            ++it;
    }
}

void RefreshAbsoluteZero(Player* player)
{
    using namespace AscensionWhiteWalkerSupport;
    bool const rootKnown = std::any_of(FrostNovaRanks.begin(), FrostNovaRanks.end(),
        [player](uint32 rank) { return player->HasActiveSpell(rank); });
    bool const active = Area52Enabled && player->HasAura(AbsoluteZero) && rootKnown;
    bool const knowsReplacement = player->HasActiveSpell(AbsoluteZeroReplacement);
    auto const guid = player->GetGUID().GetCounter();
    if (active && !knowsReplacement)
    {
        player->learnSpell(AbsoluteZeroReplacement, true);
        AbsoluteZeroGranted.insert(guid);
    }
    if (!active)
    {
        for (uint32 rank : FrostNovaRanks)
            player->SetTemporarySpellReplacement(rank, 0);
        if (AbsoluteZeroGranted.erase(guid) && player->GetSpellMap().find(AbsoluteZeroReplacement) != player->GetSpellMap().end())
            player->removeSpell(AbsoluteZeroReplacement, SPEC_MASK_ALL, true);
        return;
    }
    for (uint32 rank : FrostNovaRanks)
        player->SetTemporarySpellReplacement(rank, player->HasActiveSpell(rank) ? AbsoluteZeroReplacement : 0);
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

        if (info->SpellFamilyName == SPELLFAMILY_ROGUE && (info->SpellFamilyFlags[0] & 0x00020000) &&
            info->Effects[EFFECT_0].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
            info->Effects[EFFECT_0].BonusMultiplier = 0;

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

class aura_area52_benediction : public AuraScript
{
    PrepareAuraScript(aura_area52_benediction);

    bool _hasProc = false;
    uint32 _lastProc = 0;

    bool Load() override { return Area52Enabled; }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        return player && event.GetActor() == player && event.GetActionTarget() &&
            (event.GetTypeMask() & PROC_FLAG_KILL) && player->isHonorOrXPTarget(event.GetActionTarget()) &&
            AscensionWhiteWalkerSupport::BenedictionCooldownReady(_hasProc, getMSTimeDiff(_lastProc, getMSTime()));
    }

    void Proc(ProcEventInfo&)
    {
        _hasProc = true;
        _lastProc = getMSTime();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_benediction::CheckProc);
        OnProc += AuraProcFn(aura_area52_benediction::Proc);
    }
};

class spell_area52_benediction_party : public SpellScript
{
    PrepareSpellScript(spell_area52_benediction_party);

    std::unordered_set<ObjectGuid::LowType> _eligible;

    bool Load() override { return Area52Enabled; }

    void SelectManaTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([this](WorldObject* object)
        {
            Unit* target = object->ToUnit();
            bool const eligible = target && target != GetCaster() && target->IsAlive() &&
                !target->HasAura(811116) && AscensionWhiteWalkerSupport::BenedictionPartyManaEligible(
                    target->GetPower(POWER_MANA), target->GetMaxPower(POWER_MANA));
            if (eligible)
                _eligible.insert(target->GetGUID().GetCounter());
            return !eligible;
        });
    }

    void SelectCooldownTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([this](WorldObject* target)
        {
            return !_eligible.contains(target->GetGUID().GetCounter());
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_area52_benediction_party::SelectManaTargets, EFFECT_0, TARGET_UNIT_CASTER_AREA_PARTY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(
            spell_area52_benediction_party::SelectCooldownTargets, EFFECT_1, TARGET_UNIT_CASTER_AREA_PARTY);
    }
};

class aura_area52_cut_to_the_chase : public AuraScript
{
    PrepareAuraScript(aura_area52_cut_to_the_chase);

    bool Load() override { return Area52Enabled; }

    bool CheckProc(ProcEventInfo& event)
    {
        SpellInfo const* info = event.GetSpellInfo();
        Spell const* spell = event.GetProcSpell();
        if (!info || !info->NeedsComboPoints() || event.GetActor() != GetTarget() || !spell ||
            spell->IsTriggered() || spell->GetScriptValue(51664))
            return false;
        const_cast<Spell*>(spell)->SetScriptValue(51664, 1);
        return true;
    }

    void Proc(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        for (uint32 id : { 5171u, 6774u, 978302u })
            if (Aura* aura = owner->GetAura(id, owner->GetGUID()))
            {
                aura->SetDuration(aura->GetSpellInfo()->GetMaxDuration(), true);
                aura->SetMaxDuration(aura->GetDuration());
            }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_cut_to_the_chase::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_cut_to_the_chase::Proc, EFFECT_0, SPELL_AURA_DUMMY);
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

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;
        uint32 const spent = event.GetProcSpell() ? std::max(0, event.GetProcSpell()->GetPowerCost()) : 0;
        uint32 const preCostEnergy = player->GetPower(POWER_ENERGY) + spent;
        bool const aboveThreshold = player->GetMaxPower(POWER_ENERGY) > 0 &&
            preCostEnergy * 100 >= player->GetMaxPower(POWER_ENERGY) * 90;
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
    RegisterSpellScript(aura_area52_benediction);
    RegisterSpellScript(spell_area52_benediction_party);
    RegisterSpellScript(aura_area52_cut_to_the_chase);
    RegisterSpellScript(aura_area52_fighters_sinister_strike);
    RegisterSpellScript(aura_area52_tides_of_battle);
}
