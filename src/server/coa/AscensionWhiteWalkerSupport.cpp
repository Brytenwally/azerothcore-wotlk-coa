/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionWhiteWalkerSupportRules.h"
#include "Config.h"
#include "Creature.h"
#include "AllItemScript.h"
#include "AllSpellScript.h"
#include "Player.h"
#include "PlayerScript.h"
#include "MovementHandlerScript.h"
#include "Opcodes.h"
#include "Random.h"
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

void ApplySliceAndIceStats(SpellInfo* info, SpellInfo const* helper)
{
    if (!info || info->Id != AscensionWhiteWalkerSupport::SliceAndIce || !helper || helper->Id != 878302)
        return;
    auto const& source = helper->Effects[EFFECT_0];
    if (source.Effect != SPELL_EFFECT_APPLY_AURA || source.ApplyAuraName != SPELL_AURA_MOD_RATING_FROM_STAT)
        return;
    auto& rating = info->Effects[EFFECT_1];
    rating.Effect = SPELL_EFFECT_APPLY_AURA;
    rating.ApplyAuraName = source.ApplyAuraName;
    rating.BasePoints = source.BasePoints;
    rating.DieSides = source.DieSides;
    rating.MiscValue = source.MiscValue;
    rating.MiscValueB = source.MiscValueB;
}

void ApplyWhiteWalkerDamageCharges(SpellInfo* info)
{
    if (!info || (info->Id != 979011 && info->Id != 979053))
        return;
    bool const blades = info->Id == 979011;
    info->ProcCharges = blades ? 5 : 1;
    info->ProcFlags = blades ? PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS : PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
    info->ProcChance = 100;
    auto& proc = info->Effects[EFFECT_2];
    proc.Effect = SPELL_EFFECT_APPLY_AURA;
    proc.ApplyAuraName = SPELL_AURA_DUMMY;
    proc.BasePoints = -1;
    proc.DieSides = 1;
    proc.MiscValue = 0;
    proc.MiscValueB = 0;
    proc.TriggerSpell = 0;
    proc.SpellClassMask = blades ? flag96(131074, 0, 0) : flag96(0, 0, 32);
    proc.TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
    proc.TargetB = SpellImplicitTargetInfo();
}

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
    bool const sliceKnown = desired.contains(SliceAndIce) ||
        (player->HasActiveSpell(SliceAndIce) && !granted.contains(SliceAndIce));
    if (active && sliceKnown && player->HasActiveSpell(44545))
    {
        desired.insert(FrozenBlade);
        if (!player->HasActiveSpell(FrozenBlade))
        {
            player->learnSpell(FrozenBlade, true);
            granted.insert(FrozenBlade);
        }
    }
    uint32 iceSpear = 0;
    for (auto rank = IceSpearRanks.rbegin(); rank != IceSpearRanks.rend(); ++rank)
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(*rank);
        if ((player->HasActiveSpell(*rank) && !granted.contains(*rank)) ||
            (info && info->SpellLevel <= player->GetLevel()))
        {
            iceSpear = *rank;
            break;
        }
    }
    bool const iceLanceKnown = std::any_of(IceLanceRanks.begin(), IceLanceRanks.end(),
        [player](uint32 rank) { return player->HasActiveSpell(rank); });
    if (active && iceLanceKnown && iceSpear)
    {
        desired.insert(iceSpear);
        if (!player->HasActiveSpell(iceSpear))
        {
            player->learnSpell(iceSpear, true);
            granted.insert(iceSpear);
        }
    }
    for (uint32 rank : IceLanceRanks)
        player->SetTemporarySpellReplacement(rank,
            active && player->HasAura(WintersFury) && player->HasActiveSpell(rank) ? iceSpear : 0);
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
    white_walker_player() : PlayerScript("white_walker_player",
        {PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_UPDATE, PLAYERHOOK_ON_LOGOUT, PLAYERHOOK_ON_SPELL_CAST}) { }

    void OnPlayerLogin(Player* player) override
    {
        RefreshWhiteWalker(player);
        RefreshAbsoluteZero(player);
    }

    void OnPlayerSpellCast(Player* player, Spell* spell, bool) override
    {
        using namespace AscensionWhiteWalkerSupport;
        if (!Area52Enabled || spell->IsTriggered())
            return;
        for (uint32 auraId : { 53817u, 853817u, 853818u })
        {
            Aura const* aura = player->GetAura(auraId);
            AuraEffect const* effect = aura ? aura->GetEffect(EFFECT_0) : nullptr;
            if (effect && aura->GetStackAmount() && effect->IsAffectedOnSpell(spell->GetSpellInfo()))
            {
                spell->SetScriptValue(MaelstromWeapon, aura->GetStackAmount());
                break;
            }
        }
        uint32 const id = spell->GetSpellInfo()->Id;
        uint32 const root = sSpellMgr->GetFirstSpellInChain(id);
        if (player->HasAura(TidesOfBattle) && player->MatchesSpellInteraction(id, SinisterStrike))
        {
            spell->SetScriptValue(TidesOfBattle, uint64(player->GetPower(POWER_ENERGY)) + 1);
            spell->SetScriptValue(Ebb, player->GetMaxPower(POWER_ENERGY));
        }
        bool const winds = std::find(WindsOfWinterRanks.begin(), WindsOfWinterRanks.end(), id) != WindsOfWinterRanks.end();
        if (player->HasAura(FrozenBlade) && (root == 2098 || winds))
            spell->SetScriptValue(FrozenBlade, uint64(player->GetComboPoints()) + 1);
        if (winds && player->HasAura(WhiteWalker) && player->IsTwoHandUsed())
            spell->SetScriptValue(979052, 1);
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

        ApplyWhiteWalkerDamageCharges(info);

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
            ApplySliceAndIceStats(info, sSpellMgr->GetSpellInfo(878302));
            info->ProcFlags = PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
            info->ProcChance = 101;
            return;
        }
        if (info->Id == AscensionWhiteWalkerSupport::WintersFury)
        {
            info->Effects[EFFECT_1].Effect = 0;
            info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_NONE;
            return;
        }
        if (info->Id == AscensionWhiteWalkerSupport::FreezingBlood)
        {
            info->ProcFlags = PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS | PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
            info->ProcChance = 101;
            info->Effects[EFFECT_2].Effect = 0;
            info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_NONE;
            return;
        }
        if (info->Id == 979163)
        {
            info->ProcFlags = PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS;
            info->ProcChance = 100;
            auto& critical = info->Effects[EFFECT_1];
            critical.Effect = SPELL_EFFECT_APPLY_AURA;
            critical.ApplyAuraName = SPELL_AURA_MOD_SPELL_CRIT_CHANCE;
            critical.BasePoints = 3;
            critical.DieSides = 1;
            critical.TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
            critical.TargetB = SpellImplicitTargetInfo();
            return;
        }
        if (info->Id == AscensionWhiteWalkerSupport::FightersSinisterStrike)
        {
            info->ProcFlags = 16;
            if (SpellInfo const* helper = sSpellMgr->GetSpellInfo(781047))
                info->ProcChance = helper->ProcChance;
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
        if (!Area52Enabled || !event.GetSpellInfo() || event.GetActor() != GetTarget() ||
            !GetTarget()->ToPlayer() || !GetTarget()->ToPlayer()->IsTwoHandUsed() ||
            !GetTarget()->HasAura(AscensionWhiteWalkerSupport::SliceAndIce) ||
            !event.GetDamageInfo() || !event.GetDamageInfo()->GetDamage() ||
            !(event.GetHitMask() & PROC_HIT_CRITICAL))
            return false;
        Player* player = GetTarget()->ToPlayer();
        return player->MatchesSpellInteraction(event.GetSpellInfo()->Id, 1752) ||
            player->MatchesSpellInteraction(event.GetSpellInfo()->Id, 2098);
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* target = event.GetActionTarget();
        if (target && target->IsAlive())
            GetTarget()->CastSpell(target, 984432, TRIGGERED_FULL_MASK);
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

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->CastSpell(GetTarget(), 984937, TRIGGERED_FULL_MASK);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(984937, GetTarget()->GetGUID());
    }

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52Enabled && event.GetActor() == GetTarget() && event.GetSpellInfo() &&
            event.GetProcSpell() && !event.GetProcSpell()->IsTriggered() &&
            sSpellMgr->GetFirstSpellInChain(event.GetSpellInfo()->Id) == AscensionWhiteWalkerSupport::IceLance &&
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() &&
            (event.GetHitMask() & PROC_HIT_CRITICAL);
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::WintersVitality, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_slice_and_ice::CheckProc);
        OnProc += AuraProcFn(aura_area52_slice_and_ice::Proc);
        AfterEffectApply += AuraEffectApplyFn(aura_area52_slice_and_ice::Apply,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_slice_and_ice::Remove,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_slice_and_ice_availability : public AuraScript
{
    PrepareAuraScript(aura_area52_slice_and_ice_availability);

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52Enabled && event.GetActor() == GetTarget() && GetTarget()->ToPlayer() &&
            GetTarget()->ToPlayer()->IsTwoHandUsed() && GetTarget()->HasAura(AscensionWhiteWalkerSupport::SliceAndIce) &&
            event.GetSpellInfo() && event.GetProcSpell() && !event.GetProcSpell()->IsTriggered();
    }

    void Proc(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::WintersFury, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_slice_and_ice_availability::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_slice_and_ice_availability::Proc,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class white_walker_ice_spear : public AllSpellScript
{
public:
    white_walker_ice_spear() : AllSpellScript("white_walker_ice_spear",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        using namespace AscensionWhiteWalkerSupport;
        if (!Area52Enabled || !caster->IsPlayer() || spell->GetScriptValue(WintersFury) ||
            !caster->HasAura(WintersFury) ||
            std::find(IceSpearRanks.begin(), IceSpearRanks.end(), info->Id) == IceSpearRanks.end())
            return;
        spell->SetScriptValue(WintersFury, 1);
        caster->RemoveAurasDueToSpell(WintersFury, caster->GetGUID());
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
    {
        if (!Area52Enabled || !spell->GetScriptValue(AscensionWhiteWalkerSupport::WintersFury) ||
            !target || hit.damage <= 0 || hit.missCondition != SPELL_MISS_NONE ||
            !spell->GetSpellInfo()->IsCritCapable() || spell->GetSpellInfo()->HasAttribute(SPELL_ATTR2_CANT_CRIT) ||
            target->GetTotalAuraModifier(SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE) <= -100)
            return;
        hit.crit = true;
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool) override
    {
        using namespace AscensionWhiteWalkerSupport;
        if (!Area52Enabled || !target || miss != SPELL_MISS_NONE || !damage)
            return;
        if (uint64 const snapshot = spell->GetScriptValue(FrozenBlade))
        {
            spell->SetScriptValue(FrozenBlade, 0);
            if (roll_chance_i(std::min<uint64>(100, (snapshot - 1) * 20)))
            {
                uint32 const proc = sSpellMgr->GetFirstSpellInChain(spell->GetSpellInfo()->Id) == 2098 ?
                    FingersOfFrost : BladesOfIce;
                spell->GetCaster()->CastSpell(spell->GetCaster(), proc, TRIGGERED_FULL_MASK);
            }
        }
        if ((spell->GetScriptValue(WintersFury) || spell->GetScriptValue(979052)) && !spell->GetScriptValue(979053))
        {
            spell->SetScriptValue(979053, 1);
            spell->GetCaster()->CastSpell(spell->GetCaster(), 979053, TRIGGERED_FULL_MASK);
        }
    }
};

class aura_area52_frozen_blade_fingers_guard : public AuraScript
{
    PrepareAuraScript(aura_area52_frozen_blade_fingers_guard);

    bool Load() override { return Area52Enabled; }

    bool CheckProc(ProcEventInfo&)
    {
        return !GetTarget()->HasAura(AscensionWhiteWalkerSupport::WhiteWalker) || !GetTarget()->ToPlayer() ||
            !GetTarget()->ToPlayer()->IsTwoHandUsed() || !GetTarget()->HasAura(AscensionWhiteWalkerSupport::FrozenBlade);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_frozen_blade_fingers_guard::CheckProc);
    }
};

class spell_area52_deep_freeze : public SpellScript
{
    PrepareSpellScript(spell_area52_deep_freeze);

    bool Load() override { return Area52Enabled && GetCaster()->IsPlayer(); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({844572, 71757}); }

    void Hit(SpellEffIndex)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;
        bool const permanent = target->IsCreature() &&
            target->ToCreature()->HasMechanicTemplateImmunity(1ULL << MECHANIC_STUN);
        GetCaster()->CastSpell(target, permanent ? 71757 : 844572, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_deep_freeze::Hit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_area52_cold_blood_activation : public SpellScript
{
    PrepareSpellScript(spell_area52_cold_blood_activation);

    bool Load() override { return Area52Enabled; }

    void Activate()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !player->HasAura(AscensionWhiteWalkerSupport::FreezingBlood))
            return;
        Unit* target = player->GetSelectedUnit();
        if (!target || !target->IsAlive() || !player->IsValidAttackTarget(target))
            return;
        auto const& ranks = AscensionWhiteWalkerSupport::FrostboltRanks;
        for (auto rank = ranks.rbegin(); rank != ranks.rend(); ++rank)
            if (player->HasActiveSpell(*rank))
            {
                player->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
                player->CastSpell(target, *rank, TRIGGERED_FULL_MASK);
                return;
            }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_cold_blood_activation::Activate);
    }
};

class aura_area52_cold_blood_support : public AuraScript
{
    PrepareAuraScript(aura_area52_cold_blood_support);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({ AscensionWhiteWalkerSupport::ColdBloodSupport });
    }

    bool Load() override { return Area52Enabled && GetUnitOwner()->IsPlayer(); }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->CastSpell(GetTarget(), AscensionWhiteWalkerSupport::ColdBloodSupport, TRIGGERED_FULL_MASK);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(AscensionWhiteWalkerSupport::ColdBloodSupport, GetTarget()->GetGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_cold_blood_support::Apply,
            EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_cold_blood_support::Remove,
            EFFECT_0, SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

class movement_area52_demonic_leap : public MovementHandlerScript
{
public:
    movement_area52_demonic_leap() : MovementHandlerScript("movement_area52_demonic_leap") { }

    void OnPlayerMove(Player* player, MovementInfo, uint32 opcode) override
    {
        if (Area52Enabled && (opcode == MSG_MOVE_FALL_LAND || opcode == MSG_MOVE_START_SWIM))
            player->RemoveAurasDueToSpell(954543);
    }
};

class spell_area52_demonic_leap : public SpellScript
{
    PrepareSpellScript(spell_area52_demonic_leap);

    static constexpr uint32 WingsVisual = 954543;
    bool _reactivated = false;

    bool Load() override { return Area52Enabled && GetCaster()->IsPlayer(); }

    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({ WingsVisual }); }

    void Before()
    {
        _reactivated = GetCaster()->HasAura(AscensionWhiteWalkerSupport::DemonicLeap);
    }

    void After()
    {
        if (!_reactivated)
            GetCaster()->ToPlayer()->RemoveSpellCooldown(AscensionWhiteWalkerSupport::DemonicLeap, true);
    }

    void Visual(SpellEffIndex)
    {
        GetCaster()->CastSpell(GetCaster(), WingsVisual, TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_demonic_leap::Visual, EFFECT_2, SPELL_EFFECT_DUMMY);
        BeforeCast += SpellCastFn(spell_area52_demonic_leap::Before);
        AfterCast += SpellCastFn(spell_area52_demonic_leap::After);
    }
};

class spell_area52_roll_the_bones : public SpellScript
{
    PrepareSpellScript(spell_area52_roll_the_bones);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({ AscensionWhiteWalkerSupport::RollTheBonesLesser,
            AscensionWhiteWalkerSupport::RollTheBonesGreater });
    }

    bool Load() override { return Area52Enabled; }

    void Enhance(SpellEffIndex index)
    {
        PreventHitDefaultEffect(index);
        Unit* caster = GetCaster();
        caster->RemoveAurasDueToSpell(AscensionWhiteWalkerSupport::RollTheBonesLesser, caster->GetGUID());
        caster->RemoveAurasDueToSpell(AscensionWhiteWalkerSupport::RollTheBonesGreater, caster->GetGUID());
        caster->CastSpell(caster, AscensionWhiteWalkerSupport::RollTheBonesEnhancement(urand(0, 1)),
            TRIGGERED_FULL_MASK);
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_area52_roll_the_bones::Enhance, EFFECT_1,
            SPELL_EFFECT_ASCENSION_TRIGGER_RANDOM_SPELL);
    }
};

class aura_area52_freezing_blood : public AuraScript
{
    PrepareAuraScript(aura_area52_freezing_blood);

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!Area52Enabled || !player || !event.GetSpellInfo() || event.GetActor() != player)
            return false;
        auto const* damage = event.GetDamageInfo();
        return damage && damage->GetDamage() && (event.GetSchoolMask() & SPELL_SCHOOL_MASK_FROST) &&
            !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC);
    }

    void Proc(ProcEventInfo&)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;
        uint32 const now = getMSTime();
        auto const guid = player->GetGUID().GetCounter();
        auto const last = FreezingBloodLastCooldown.find(guid);
        if (last != FreezingBloodLastCooldown.end() && getMSTimeDiff(last->second, now) < 1000)
            return;
        FreezingBloodLastCooldown[guid] = now;
        player->ModifySpellCooldown(AscensionWhiteWalkerSupport::ColdBlood, -10000);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(979163, GetTarget()->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_freezing_blood::CheckProc);
        OnProc += AuraProcFn(aura_area52_freezing_blood::Proc);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_freezing_blood::Remove,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_freezing_blood_consumed : public AuraScript
{
    PrepareAuraScript(aura_area52_freezing_blood_consumed);

    bool Load() override { return Area52Enabled; }

    void Consumed(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* owner = GetTarget();
        if (!owner->HasAura(AscensionWhiteWalkerSupport::FreezingBlood))
            return;
        SpellInfo const* source = sSpellMgr->GetSpellInfo(AscensionWhiteWalkerSupport::FreezingBlood);
        if (!source)
            return;
        owner->CastCustomSpell(979163, SPELLVALUE_BASE_POINT1,
            source->Effects[EFFECT_2].CalcValue(owner), owner, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_freezing_blood_consumed::Consumed,
            EFFECT_0, SPELL_AURA_MOD_WEAPON_CRIT_PERCENT, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
    }
};

class aura_area52_freezing_blood_damage : public AuraScript
{
    PrepareAuraScript(aura_area52_freezing_blood_damage);

    bool Load() override { return Area52Enabled; }

    bool CheckProc(ProcEventInfo& event)
    {
        SpellInfo const* info = event.GetSpellInfo();
        auto const* damage = event.GetDamageInfo();
        return info && event.GetActor() == GetTarget() && damage && damage->GetDamage() &&
            !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) && info->SpellFamilyName == SPELLFAMILY_ROGUE &&
            info->DmgClass == SPELL_DAMAGE_CLASS_MELEE &&
            (info->NeedsComboPoints() || info->HasEffect(SPELL_EFFECT_ADD_COMBO_POINTS));
    }

    void Proc(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* victim = event.GetActionTarget();
        SpellInfo const* bonus = sSpellMgr->GetSpellInfo(979134);
        if (!victim || !victim->IsAlive() || !bonus || !event.GetDamageInfo())
            return;
        int32 const amount = CalculatePct(event.GetDamageInfo()->GetDamage(),
            bonus->Effects[EFFECT_0].CalcValue(GetTarget()));
        GetTarget()->CastCustomSpell(979134, SPELLVALUE_BASE_POINT0, amount, victim, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_freezing_blood_damage::CheckProc);
        OnEffectProc += AuraEffectProcFn(aura_area52_freezing_blood_damage::Proc,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
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
        return Area52Enabled && event.GetActor() == GetTarget() && event.GetSpellInfo() &&
            GetTarget()->ToPlayer() && GetTarget()->ToPlayer()->MatchesSpellInteraction(
                event.GetSpellInfo()->Id, AscensionWhiteWalkerSupport::SinisterStrike) &&
            !GetTarget()->HasAura(780999) && event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        int32 attackPower = GetTarget()->GetLevel() * GetStackAmount();
        int32 energy = 10 * GetStackAmount();
        int32 rage = energy * 10;
        GetTarget()->CastCustomSpell(GetTarget(), AscensionWhiteWalkerSupport::FightersPower,
            &attackPower, &attackPower, nullptr, true);
        GetTarget()->CastCustomSpell(GetTarget(), 781001, &energy, nullptr, nullptr, true);
        GetTarget()->CastCustomSpell(GetTarget(), 781002, &rage, nullptr, nullptr, true);
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
        return Area52Enabled && event.GetActor() == GetTarget() && event.GetSpellInfo() &&
            GetTarget()->ToPlayer() && GetTarget()->ToPlayer()->MatchesSpellInteraction(
                event.GetSpellInfo()->Id, AscensionWhiteWalkerSupport::SinisterStrike) &&
            GetTarget()->ToPlayer() && event.GetProcSpell() &&
            event.GetProcSpell()->GetScriptValue(AscensionWhiteWalkerSupport::TidesOfBattle) &&
            !event.GetProcSpell()->GetScriptValue(AscensionWhiteWalkerSupport::Flow) &&
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
    }

    void Proc(ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;
        Spell const* source = event.GetProcSpell();
        uint64 const preCostEnergy = source->GetScriptValue(AscensionWhiteWalkerSupport::TidesOfBattle) - 1;
        uint64 const maximum = source->GetScriptValue(AscensionWhiteWalkerSupport::Ebb);
        bool const aboveThreshold = maximum > 0 && preCostEnergy * 100 >= maximum * 90;
        const_cast<Spell*>(source)->SetScriptValue(AscensionWhiteWalkerSupport::Flow, 1);
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

class aura_area52_ebb_drain : public AuraScript
{
    PrepareAuraScript(aura_area52_ebb_drain);

    bool Load() override { return Area52Enabled && GetUnitOwner()->IsPlayer(); }

    void Tick(AuraEffect const*)
    {
        PreventDefaultAction();
        int32 const percent = std::max(0, -GetEffect(EFFECT_1)->GetAmount());
        int32 const drain = int64(GetTarget()->GetMaxPower(POWER_ENERGY)) * percent / 100;
        GetTarget()->ModifyPower(POWER_ENERGY, -drain);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_ebb_drain::Tick,
            EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

}

void AddSC_AscensionWhiteWalkerSupport()
{
    new white_walker_ice_spear();
    new white_walker_support_metadata();
    new white_walker_player();
    new player_area52_frostbrand();
    RegisterSpellScript(spell_area52_honed_ice);
    new item_area52_frostbrand();
    RegisterSpellScript(aura_area52_white_walker);
    RegisterSpellScript(aura_area52_slice_and_ice);
    RegisterSpellScript(aura_area52_slice_and_ice_availability);
    RegisterSpellScript(spell_area52_deep_freeze);
    RegisterSpellScript(aura_area52_frozen_blade_fingers_guard);
    RegisterSpellScript(aura_area52_freezing_blood);
    RegisterSpellScript(spell_area52_cold_blood_activation);
    RegisterSpellScript(aura_area52_cold_blood_support);
    RegisterSpellScript(spell_area52_demonic_leap);
    new movement_area52_demonic_leap();
    RegisterSpellScript(spell_area52_roll_the_bones);
    RegisterSpellScript(aura_area52_freezing_blood_consumed);
    RegisterSpellScript(aura_area52_freezing_blood_damage);
    RegisterSpellScript(aura_area52_benediction);
    RegisterSpellScript(spell_area52_benediction_party);
    RegisterSpellScript(aura_area52_cut_to_the_chase);
    RegisterSpellScript(aura_area52_fighters_sinister_strike);
    RegisterSpellScript(aura_area52_tides_of_battle);
    RegisterSpellScript(aura_area52_ebb_drain);
}
