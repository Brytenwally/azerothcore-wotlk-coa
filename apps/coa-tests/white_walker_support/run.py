import argparse
from pathlib import Path
import shutil
import sqlite3
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]

FROZEN_POWER_MOCKS = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <array>
#include <cassert>
using uint32 = std::uint32_t;
constexpr int EFFECT_0 = 0;
constexpr int SPELL_EFFECT_APPLY_AURA = 6;
constexpr int SPELL_AURA_ADD_FLAT_MODIFIER = 107;
constexpr int SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE = 303;
constexpr int ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE = 20012;
constexpr int TARGET_UNIT_CASTER = 1;
struct flag96
{
    std::array<uint32, 3> values;
    flag96(uint32 a, uint32 b, uint32 c) : values{a, b, c} {}
    bool operator==(flag96 const&) const = default;
};
struct Target { int value; int GetTarget() const { return value; } };
struct SpellEffectInfo
{
    int Effect = SPELL_EFFECT_APPLY_AURA;
    int ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
    int MiscValue = 23;
    int MiscValueB = 0;
    flag96 SpellClassMask{16777216, 0, 0};
    Target TargetA{TARGET_UNIT_CASTER};
    Target TargetB{0};
    int BasePoints = 99;
    int DieSides = 1;
};
struct SpellInfo { uint32 Id; std::array<SpellEffectInfo, 3> Effects; };
'''

FROZEN_POWER_CASES = r'''
int main()
{
    std::array<uint32, 4> ids{63373, 63374, 263374, 414600};
    std::array<int, 4> percentages{3, 6, 9, 2};
    for (std::size_t index = 0; index < ids.size(); ++index)
    {
        SpellInfo info{ids[index], {}};
        ApplyFrozenPowerDamageContract(&info);
        auto const& effect = info.Effects[0];
        assert(effect.BasePoints + effect.DieSides == percentages[index]);
        assert(effect.ApplyAuraName == SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE);
        assert(effect.MiscValueB == ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE);
        if (ids[index] == 414600)
            for (int copies = 1; copies <= 3; ++copies)
                assert((effect.BasePoints + effect.DieSides) * copies == 2 * copies);
    }
    SpellInfo unrelated{123, {}};
    ApplyFrozenPowerDamageContract(&unrelated);
    assert(unrelated.Effects[0].BasePoints == 99);
    SpellInfo wrongShape{63373, {}};
    wrongShape.Effects[0].TargetA.value = 6;
    ApplyFrozenPowerDamageContract(&wrongShape);
    assert(wrongShape.Effects[0].BasePoints == 99);
    ApplyFrozenPowerDamageContract(nullptr);
}
'''

BENEDICTION_MOCKS = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <cassert>
#include <list>
#include <unordered_set>
#include <map>
using uint32 = std::uint32_t;
using int32 = std::int32_t;
uint32 clockTime = 0;
uint32 getMSTime() { return clockTime; }
uint32 getMSTimeDiff(uint32 first, uint32 second) { return second - first; }
bool Area52Enabled = true;
constexpr uint32 PROC_FLAG_KILL = 1;
constexpr uint32 PROC_FLAG_DONE_PERIODIC = 0x40000;
constexpr uint32 SPELL_SCHOOL_MASK_FROST = 16;
constexpr int SPELL_AURA_MOD_STEALTH = 16;
constexpr int TRIGGERED_FULL_MASK = 1;
constexpr int EFFECT_0 = 0;
constexpr int EFFECT_2 = 2;
constexpr int SPELLVALUE_BASE_POINT0 = 0;
constexpr int SPELLVALUE_BASE_POINT1 = 1;
constexpr int SPELLFAMILY_ROGUE = 8;
constexpr int SPELL_DAMAGE_CLASS_MELEE = 2;
constexpr int SPELL_EFFECT_ADD_COMBO_POINTS = 80;
using AuraEffectHandleModes = int;
int32 CalculatePct(uint32 amount, int32 percent) { return amount * percent / 100; }
struct Player;
struct Unit;
struct ObjectGuid
{
    using LowType = uint32;
    uint32 id;
    uint32 GetCounter() const { return id; }
    bool operator==(ObjectGuid const&) const = default;
};
struct EffectData { int32 value = 4; int32 CalcValue(Unit*) const { return value; } };
struct SpellInfo
{
    bool finishing = true;
    bool generating = false;
    uint32 SpellFamilyName = 8;
    int DmgClass = SPELL_DAMAGE_CLASS_MELEE;
    std::array<EffectData, 3> Effects;
    int32 maximumDuration = 21000;
    bool NeedsComboPoints() const { return finishing; }
    int32 GetMaxDuration() const { return maximumDuration; }
    bool HasEffect(int effect) const { return effect == SPELL_EFFECT_ADD_COMBO_POINTS && generating; }
};
struct SpellMgr
{
    SpellInfo parent;
    SpellInfo bonus;
    SpellInfo const* GetSpellInfo(uint32 id) const { return id == 979135 ? &parent : &bonus; }
};
SpellMgr spellMgr;
SpellMgr* sSpellMgr = &spellMgr;
struct Spell
{
    bool triggered = false;
    mutable std::map<uint32, uint32> values;
    bool IsTriggered() const { return triggered; }
    uint32 GetScriptValue(uint32 id) const { auto it = values.find(id); return it == values.end() ? 0 : it->second; }
    void SetScriptValue(uint32 id, uint32 value) { values[id] = value; }
    bool TryMarkScriptEventHandled(uint32 id) const
    {
        if (id >= 32)
            return false;
        if (values.contains(id))
            return false;
        values[id] = 1;
        return true;
    }
};
struct AuraEffect {};
struct Aura
{
    SpellInfo info;
    int32 duration = 3000;
    int32 maximum = 3000;
    SpellInfo const* GetSpellInfo() const { return &info; }
    void SetDuration(int32 value, bool) { duration = value; }
    void SetMaxDuration(int32 value) { maximum = value; }
    int32 GetDuration() const { return duration; }
};
struct WorldObject
{
    virtual Unit* ToUnit() { return nullptr; }
    virtual ObjectGuid GetGUID() const { return {0}; }
};
constexpr int POWER_MANA = 0;
struct Unit : WorldObject
{
    Player* player = nullptr;
    uint32 id = 1;
    uint32 mana = 0;
    uint32 maximum = 100;
    bool alive = true;
    bool cooldown = false;
    bool freezing = false;
    uint32 customId = 0;
    int customField = 0;
    int32 customAmount = 0;
    Unit* customTarget = nullptr;
    uint32 removedId = 0;
    std::map<uint32, Aura*> auraMap;
    Player* ToPlayer() { return player; }
    Unit* ToUnit() override { return this; }
    ObjectGuid GetGUID() const override { return {id}; }
    bool IsAlive() const { return alive; }
    bool HasAura(uint32 id) const { return id == 979135 ? freezing : cooldown; }
    uint32 GetPower(int) const { return mana; }
    uint32 GetMaxPower(int) const { return maximum; }
    Aura* GetAura(uint32 id, ObjectGuid guid)
    {
        auto it = auraMap.find(id);
        return guid == GetGUID() && it != auraMap.end() ? it->second : nullptr;
    }
    void CastCustomSpell(uint32 id, int field, int32 amount, Unit* target, bool)
    {
        customId = id;
        customField = field;
        customAmount = amount;
        customTarget = target;
    }
    void RemoveAurasDueToSpell(uint32 id, ObjectGuid guid)
    {
        if (guid == GetGUID())
        {
            removedId = id;
            auraMap.erase(id);
        }
    }
};
struct Player : Unit
{
    bool eligible = true;
    bool attackAllowed = true;
    bool stealthRemoved = false;
    Unit* selected = nullptr;
    Unit* castTarget = nullptr;
    uint32 castId = 0;
    int cooldownAdjustment = 0;
    std::unordered_set<uint32> knownSpells;
    Player() { player = this; }
    bool isHonorOrXPTarget(Unit*) const { return eligible; }
    bool HasAura(uint32 id) const { return id == 979135 ? freezing : Unit::HasAura(id); }
    bool HasActiveSpell(uint32 id) const { return knownSpells.contains(id); }
    Unit* GetSelectedUnit() const { return selected; }
    bool IsValidAttackTarget(Unit*) const { return attackAllowed; }
    void RemoveAurasByType(int) { stealthRemoved = true; }
    void CastSpell(Unit* target, uint32 id, int) { castTarget = target; castId = id; }
    void ModifySpellCooldown(uint32, int value) { cooldownAdjustment += value; }
};
Player owner;
std::map<uint32, uint32> FreezingBloodLastCooldown;
struct AuraScript
{
    virtual bool Load() { return true; }
    Unit* GetTarget() { return &owner; }
    void PreventDefaultAction() {}
};
struct SpellScript
{
    virtual bool Load() { return true; }
    Unit* GetCaster() { return &owner; }
};
struct DamageInfo { uint32 amount; uint32 GetDamage() const { return amount; } };
struct ProcEventInfo
{
    Unit* actor;
    Unit* target;
    uint32 mask = PROC_FLAG_KILL;
    SpellInfo* info = nullptr;
    Spell* spell = nullptr;
    DamageInfo* damage = nullptr;
    uint32 school = SPELL_SCHOOL_MASK_FROST;
    Unit* GetActor() const { return actor; }
    Unit* GetActionTarget() const { return target; }
    uint32 GetTypeMask() const { return mask; }
    SpellInfo const* GetSpellInfo() const { return info; }
    Spell* GetProcSpell() const { return spell; }
    DamageInfo const* GetDamageInfo() const { return damage; }
    uint32 GetSchoolMask() const { return school; }
};
#define PrepareAuraScript(name) public:
#define PrepareSpellScript(name) public:
'''

BENEDICTION_CASES = r'''
int main()
{
    aura_area52_benediction aura;
    Unit victim;
    ProcEventInfo event{&owner, &victim};
    assert(aura.Load());
    assert(aura.CheckProc(event));
    aura.Proc(event);
    assert(!aura.CheckProc(event));
    clockTime = 4999;
    assert(!aura.CheckProc(event));
    clockTime = 5000;
    assert(aura.CheckProc(event));
    event.mask = 0;
    assert(!aura.CheckProc(event));
    event.mask = PROC_FLAG_KILL;
    owner.eligible = false;
    assert(!aura.CheckProc(event));
    owner.eligible = true;
    event.actor = &victim;
    assert(!aura.CheckProc(event));
    event.actor = &owner;
    event.target = nullptr;
    assert(!aura.CheckProc(event));
    spell_area52_benediction_party party;
    Unit ally, boundary, dead, guarded, noMana;
    ally.id = 2;
    ally.mana = 49;
    boundary.id = 3;
    boundary.mana = 50;
    dead.id = 4;
    dead.alive = false;
    guarded.id = 5;
    guarded.cooldown = true;
    noMana.id = 6;
    noMana.maximum = 0;
    std::list<WorldObject*> targets{&owner, &ally, &boundary, &dead, &guarded, &noMana};
    party.SelectManaTargets(targets);
    assert(targets.size() == 1 && targets.front() == &ally);
    ally.mana = 79;
    std::list<WorldObject*> cooldownTargets{&owner, &ally, &boundary, &dead, &guarded, &noMana};
    party.SelectCooldownTargets(cooldownTargets);
    assert(cooldownTargets.size() == 1 && cooldownTargets.front() == &ally);
    aura_area52_cut_to_the_chase cut;
    SpellInfo finisher;
    Spell cast;
    Aura slice, ice;
    owner.auraMap[5171] = &slice;
    owner.auraMap[978302] = &ice;
    event = ProcEventInfo{&owner, &victim};
    event.info = &finisher;
    event.spell = &cast;
    finisher.finishing = false;
    assert(!cut.CheckProc(event));
    finisher.finishing = true;
    cast.triggered = true;
    assert(!cut.CheckProc(event));
    cast.triggered = false;
    assert(cut.CheckProc(event));
    assert(!cut.CheckProc(event));
    cut.Proc(nullptr, event);
    assert(slice.duration == 21000 && slice.maximum == 21000);
    assert(ice.duration == 21000 && ice.maximum == 21000);
    spell_area52_cold_blood_activation activation;
    owner.freezing = true;
    owner.selected = &victim;
    for (uint32 rank : AscensionWhiteWalkerSupport::FrostboltRanks)
    {
        owner.knownSpells.clear();
        owner.knownSpells.insert(rank);
        owner.castId = 0;
        activation.Activate();
        assert(owner.castTarget == &victim && owner.castId == rank);
        assert(owner.stealthRemoved);
    }
    owner.knownSpells.insert(116);
    activation.Activate();
    assert(owner.castId == 42842);
    owner.castId = 0;
    owner.attackAllowed = false;
    activation.Activate();
    assert(owner.castId == 0);
    owner.attackAllowed = true;
    owner.selected = nullptr;
    activation.Activate();
    assert(owner.castId == 0);
    owner.selected = &victim;
    owner.knownSpells.clear();
    activation.Activate();
    assert(owner.castId == 0);
    aura_area52_freezing_blood freezing;
    DamageInfo hit{100};
    event.damage = &hit;
    clockTime = 0;
    FreezingBloodLastCooldown.clear();
    assert(freezing.CheckProc(event));
    freezing.Proc(event);
    freezing.Proc(event);
    assert(owner.cooldownAdjustment == -10000);
    clockTime = 999;
    freezing.Proc(event);
    assert(owner.cooldownAdjustment == -10000);
    clockTime = 1000;
    freezing.Proc(event);
    assert(owner.cooldownAdjustment == -20000);
    event.mask = PROC_FLAG_DONE_PERIODIC;
    assert(!freezing.CheckProc(event));
    event.mask = 0;
    event.school = 4;
    assert(!freezing.CheckProc(event));
    event.school = SPELL_SCHOOL_MASK_FROST;
    event.actor = &victim;
    assert(!freezing.CheckProc(event));
    aura_area52_freezing_blood_consumed consumed;
    owner.freezing = true;
    consumed.Consumed(nullptr, 0);
    assert(owner.customId == 979163 && owner.customField == 1 && owner.customAmount == 4);
    assert(owner.customTarget == &owner);
    owner.freezing = false;
    owner.customId = 0;
    consumed.Consumed(nullptr, 0);
    assert(owner.customId == 0);
    owner.freezing = true;
    aura_area52_freezing_blood_damage bonus;
    event.actor = &owner;
    event.mask = 0;
    event.school = SPELL_SCHOOL_MASK_FROST;
    spellMgr.bonus.Effects[0].value = 20;
    assert(bonus.CheckProc(event));
    bonus.Proc(nullptr, event);
    assert(owner.customId == 979134 && owner.customField == 0 && owner.customAmount == 20);
    assert(owner.customTarget == &victim);
    finisher.finishing = false;
    assert(!bonus.CheckProc(event));
    finisher.generating = true;
    assert(bonus.CheckProc(event));
    finisher.DmgClass = 1;
    assert(!bonus.CheckProc(event));
    finisher.DmgClass = SPELL_DAMAGE_CLASS_MELEE;
    event.mask = PROC_FLAG_DONE_PERIODIC;
    assert(!bonus.CheckProc(event));
    owner.auraMap[979163] = &ice;
    freezing.Remove(nullptr, 0);
    assert(owner.removedId == 979163 && !owner.auraMap.contains(979163));
    Area52Enabled = false;
    assert(!aura.Load());
}
'''

GRANT_MOCKS = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <cassert>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
using uint32 = std::uint32_t;
struct ObjectGuid
{
    using LowType = uint32;
    uint32 GetCounter() const { return 1; }
};
constexpr int SPEC_MASK_ALL = 0;
bool Area52Enabled = true;
std::unordered_map<uint32, std::unordered_set<uint32>> WhiteWalkerGranted;
std::unordered_set<uint32> AbsoluteZeroGranted;
struct Player
{
    bool active = true;
    bool twoHanded = true;
    std::map<uint32, bool> spells;
    std::map<uint32, uint32> replacements;
    ObjectGuid GetGUID() const { return {}; }
    bool HasAura(uint32) const { return active; }
    bool IsTwoHandUsed() const { return twoHanded; }
    bool HasActiveSpell(uint32 id) const { return spells.contains(id); }
    auto const& GetSpellMap() const { return spells; }
    void learnSpell(uint32 id, bool) { spells[id] = true; }
    void removeSpell(uint32 id, int, bool) { spells.erase(id); }
    void SetTemporarySpellReplacement(uint32 id, uint32 replacement) { replacements[id] = replacement; }
};
'''

GRANT_CASES = r'''
int main()
{
    using namespace AscensionWhiteWalkerSupport;
    Player player;
    player.spells[IceLance] = true;
    RefreshWhiteWalker(&player);
    assert(player.HasActiveSpell(IceSpear));
    assert(WhiteWalkerGranted[1].contains(IceSpear));
    RefreshWhiteWalker(&player);
    assert(WhiteWalkerGranted[1].contains(IceSpear));
    player.spells.erase(IceLance);
    RefreshWhiteWalker(&player);
    assert(!player.HasActiveSpell(IceSpear));
    player.spells[IceLance] = true;
    RefreshWhiteWalker(&player);
    player.twoHanded = false;
    RefreshWhiteWalker(&player);
    assert(!player.HasActiveSpell(IceSpear));
    player.twoHanded = true;
    player.spells[IceSpear] = true;
    RefreshWhiteWalker(&player);
    player.active = false;
    RefreshWhiteWalker(&player);
    assert(player.HasActiveSpell(IceSpear));
    WhiteWalkerGranted.clear();
    player = Player{};
    for (std::size_t rank = 0; rank < ConeOfColdRanks.size(); ++rank)
    {
        if (rank)
            player.spells.erase(ConeOfColdRanks[rank - 1]);
        player.spells[ConeOfColdRanks[rank]] = true;
        RefreshWhiteWalker(&player);
        assert(player.HasActiveSpell(WindsOfWinterRanks[rank]));
        assert(player.replacements[ConeOfColdRanks[rank]] == WindsOfWinterRanks[rank]);
        RefreshWhiteWalker(&player);
        assert(WhiteWalkerGranted[1].contains(WindsOfWinterRanks[rank]));
        if (rank)
            assert(!player.HasActiveSpell(WindsOfWinterRanks[rank - 1]));
    }
    player.twoHanded = false;
    RefreshWhiteWalker(&player);
    for (uint32 rank : WindsOfWinterRanks)
        assert(!player.HasActiveSpell(rank));
    for (uint32 rank : SliceAndDiceRanks)
    {
        player = Player{};
        WhiteWalkerGranted.clear();
        player.spells[rank] = true;
        RefreshWhiteWalker(&player);
        assert(player.HasActiveSpell(SliceAndIce));
        assert(player.replacements[rank] == SliceAndIce);
        player.spells.erase(rank);
        RefreshWhiteWalker(&player);
        assert(!player.HasActiveSpell(SliceAndIce));
    }
    for (uint32 rank : IceLanceRanks)
    {
        player = Player{};
        WhiteWalkerGranted.clear();
        player.spells[rank] = true;
        RefreshWhiteWalker(&player);
        assert(player.HasActiveSpell(IceSpear));
        player.spells.erase(rank);
        RefreshWhiteWalker(&player);
        assert(!player.HasActiveSpell(IceSpear));
    }
    for (uint32 rank : FrostNovaRanks)
    {
        player = Player{};
        AbsoluteZeroGranted.clear();
        player.spells[rank] = true;
        RefreshAbsoluteZero(&player);
        assert(player.HasActiveSpell(AbsoluteZeroReplacement));
        assert(player.replacements[rank] == AbsoluteZeroReplacement);
        RefreshAbsoluteZero(&player);
        assert(AbsoluteZeroGranted.contains(1));
        player.spells.erase(rank);
        RefreshAbsoluteZero(&player);
        assert(!player.HasActiveSpell(AbsoluteZeroReplacement));
        assert(player.replacements[rank] == 0);
    }
}
'''

MAIN = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <cassert>
#include <algorithm>
#include <vector>
#include <map>
#include <list>
using namespace AscensionWhiteWalkerSupport;
int main()
{
    static_assert(AthleticCostPercent == -4 && AthleticSchools == 17);
    assert(BenedictionCooldownReady(false, 0));
    assert(!BenedictionCooldownReady(true, 4999));
    assert(BenedictionCooldownReady(true, 5000));
    assert(BenedictionCooldownReady(true, 5001));
    assert(BenedictionPartyManaEligible(49, 100));
    assert(!BenedictionPartyManaEligible(50, 100));
    assert(!BenedictionPartyManaEligible(51, 100));
    assert(!BenedictionPartyManaEligible(0, 0));
    assert(BenedictionPartyManaEligible(0, 100));
    static_assert(WindsOfWinterRanks.size() == 8 && WindsOfWinterRanks.front() == 978846 && WindsOfWinterRanks.back() == 978853);
    static_assert(IceSpearRanks.size() == 7 && IceSpearRanks.front() == 984935 && IceSpearRanks.back() == 984975);
    static_assert(WintersFury == 984936 && WintersVitality == 978525);
    static_assert(FrostbrandDamageHelpers.size() == 9 && FrostbrandDamageHelpers.front() == 965990 && FrostbrandDamageHelpers.back() == 965998);
    static_assert(FrostbrandSlowHelpers.size() == 9 && FrostbrandSlowHelpers.front() == 8034 && FrostbrandSlowHelpers.back() == 58799);
    static_assert(FrozenPowerTalentPercent(0) == 3 && FrozenPowerTalentPercent(1) == 6 && FrozenPowerTalentPercent(2) == 9);
    static_assert(FrozenPowerMysticPercent(1) == 2 && FrozenPowerMysticPercent(2) == 4 && FrozenPowerMysticPercent(3) == 6 && FrozenPowerMysticPercent(4) == 6);
    static_assert(FrozenPowerSchoolAllowed(4) && FrozenPowerSchoolAllowed(8) && FrozenPowerSchoolAllowed(16));
    static_assert(!FrozenPowerSchoolAllowed(1) && !FrozenPowerSchoolAllowed(32));
    static_assert(EviscerateClientCoefficient(1) == 2 && EviscerateClientCoefficient(5) == 50);
    constexpr std::array<int, 6> expectedEviscerate = {0, 30, 120, 270, 480, 750};
    for (std::uint8_t points = 0; points <= 5; ++points)
    {
        assert(EviscerateScalingBonus(true, 1000, 500, points) == expectedEviscerate[points]);
        assert(EviscerateScalingBonus(false, 1000, 500, points) == 70 * points);
        assert(EviscerateScalingBonus(true, 0, 0, points) == 0);
    }
    assert(HonedIceSource(3, 512, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 16, true, 1, true));
    assert(!HonedIceSource(3, 512, 16, false, 0, true));
    assert(!HonedIceSource(3, 512, 16, false, 1, false));
    assert(!HonedIceSource(3, 1, 16, false, 1, true));
    assert(!HonedIceSource(3, 512, 4, false, 1, true));
    assert(!HonedIceSource(8, 512, 16, false, 1, true));
    assert(HonedIceCriticalBonus(2, 1) == 2);
    assert(HonedIceCriticalBonus(2, 3) == 6);
    assert(HonedIceCriticalBonus(2, 0) == 0);
    assert(WhiteWalkerActive(true, true, true));
    assert(!WhiteWalkerActive(false, true, true));
    assert(!WhiteWalkerActive(true, false, true));
    assert(!WhiteWalkerActive(true, true, false));
}
'''

FROSTBRAND_MOCKS = r'''
using uint32 = std::uint32_t;
struct Unit {};
struct ObjectGuid { int value = 0; bool operator==(ObjectGuid const& other) const { return value == other.value; } };
enum EnchantmentSlot { TEMP_ENCHANTMENT_SLOT, OTHER_SLOT };
struct Item
{
    ObjectGuid guid;
    uint32 enchantment = 2;
    ObjectGuid GetGUID() const { return guid; }
    uint32 GetEnchantmentId(EnchantmentSlot) const { return enchantment; }
};
struct SpellInfo { uint32 Id; };
constexpr int TRIGGERED_FULL_MASK = 1;
bool Area52Enabled = true;
struct Aura { ObjectGuid guid; ObjectGuid GetCastItemGUID() const { return guid; } };
struct AuraApplication { Aura* aura; Aura* GetBase() const { return aura; } };
struct PlayerScript
{
    explicit PlayerScript(char const*) {}
    virtual bool OnPlayerCanApplyEnchantment(struct Player*, Item*, EnchantmentSlot, bool, bool, bool) { return true; }
};
struct AllItemScript
{
    explicit AllItemScript(char const*) {}
    virtual bool OnItemCombatSpell(struct Player*, Unit*, SpellInfo const*, Item*) { return true; }
};
struct Player : Unit
{
    struct Cast { Unit* target; uint32 spell; Item* item; };
    std::vector<Cast> casts;
    std::multimap<uint32, AuraApplication*> auras;
    std::list<Aura> auraStorage;
    std::list<AuraApplication> applicationStorage;
    auto const& GetAppliedAuras() const { return auras; }
    void CastSpell(Unit* target, uint32 spell, int, Item* item)
    {
        casts.push_back({target, spell, item});
        if (target == this)
        {
            auraStorage.push_back({item->GetGUID()});
            applicationStorage.push_back({&auraStorage.back()});
            auras.emplace(spell, &applicationStorage.back());
        }
    }
    void RemoveAurasDueToItemSpell(uint32 spell, ObjectGuid guid)
    {
        for (auto it = auras.begin(); it != auras.end();)
            if (it->first == spell && it->second->GetBase()->GetCastItemGUID() == guid)
                it = auras.erase(it);
            else
                ++it;
    }
};
'''

FROSTBRAND_CASES = r'''
    item_area52_frostbrand handler;
    Player player;
    Unit victim;
    Item weapon;
    constexpr std::array<uint32, 9> helpers = {965990,965991,965992,965993,965994,965995,965996,965997,965998};
    constexpr std::array<uint32, 9> attacks = {8034,8037,10458,16352,16353,965999,58797,58798,58799};
    for (std::size_t rank = 0; rank < helpers.size(); ++rank)
    {
        SpellInfo spell{helpers[rank]};
        assert(handler.OnItemCombatSpell(&player, &victim, &spell, &weapon));
        assert(player.casts.size() == rank + 1);
        assert(player.casts.back().target == &victim);
        assert(player.casts.back().spell == attacks[rank]);
        assert(player.casts.back().item == &weapon);
    }
    for (uint32 id : {0u, 965989u, 965999u, 58799u})
    {
        SpellInfo unrelated{id};
        assert(handler.OnItemCombatSpell(&player, &victim, &unrelated, &weapon));
    }
    SpellInfo helper{965990};
    assert(handler.OnItemCombatSpell(nullptr, &victim, &helper, &weapon));
    assert(handler.OnItemCombatSpell(&player, nullptr, &helper, &weapon));
    assert(handler.OnItemCombatSpell(&player, &victim, nullptr, &weapon));
    Area52Enabled = false;
    assert(handler.OnItemCombatSpell(&player, &victim, &helper, &weapon));
    assert(player.casts.size() == 9);
    Area52Enabled = true;
    player_area52_frostbrand lifecycle;
    Player equipped;
    Item mainHand{{1}, 2}, offHand{{2}, 2};
    for (std::size_t rank = 0; rank < FrostbrandEnchantments.size(); ++rank)
    {
        mainHand.enchantment = FrostbrandEnchantments[rank];
        offHand.enchantment = FrostbrandEnchantments[rank];
        assert(lifecycle.OnPlayerCanApplyEnchantment(&equipped, &mainHand, TEMP_ENCHANTMENT_SLOT, true, true, false));
        assert(lifecycle.OnPlayerCanApplyEnchantment(&equipped, &offHand, TEMP_ENCHANTMENT_SLOT, true, true, false));
        assert(equipped.auras.size() == 2);
        assert(player_area52_frostbrand::HasItemHelper(&equipped, helpers[rank], mainHand.GetGUID()));
        assert(player_area52_frostbrand::HasItemHelper(&equipped, helpers[rank], offHand.GetGUID()));
        auto count = equipped.casts.size();
        lifecycle.OnPlayerCanApplyEnchantment(&equipped, &mainHand, TEMP_ENCHANTMENT_SLOT, true, true, false);
        assert(equipped.casts.size() == count);
        lifecycle.OnPlayerCanApplyEnchantment(&equipped, &mainHand, TEMP_ENCHANTMENT_SLOT, false, true, false);
        assert(equipped.auras.size() == 1);
        assert(player_area52_frostbrand::HasItemHelper(&equipped, helpers[rank], offHand.GetGUID()));
        lifecycle.OnPlayerCanApplyEnchantment(&equipped, &mainHand, TEMP_ENCHANTMENT_SLOT, true, true, false);
        assert(equipped.auras.size() == 2);
    }
    auto count = equipped.casts.size();
    lifecycle.OnPlayerCanApplyEnchantment(&equipped, &mainHand, OTHER_SLOT, true, true, false);
    assert(equipped.casts.size() == count);
'''

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dbc-dir')
    parser.parse_args()
    source = (ROOT / 'src/server/coa/AscensionWhiteWalkerSupport.cpp').read_text()
    migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000001_area52_benediction.sql').read_text()
    with sqlite3.connect(':memory:') as database:
        database.execute('CREATE TABLE spell_script_names (spell_id INTEGER, ScriptName TEXT)')
        database.execute("INSERT INTO spell_script_names VALUES (20101, 'unrelated')")
        database.executescript(migration)
        database.executescript(migration)
        rows = database.execute('SELECT spell_id, ScriptName FROM spell_script_names ORDER BY spell_id, ScriptName').fetchall()
        assert rows == [(20101, 'aura_area52_benediction'), (20101, 'unrelated'),
                        (20102, 'aura_area52_benediction'), (20103, 'aura_area52_benediction'),
                        (411116, 'spell_area52_benediction_party')]
        database.execute('CREATE TABLE spell_proc (SpellId INTEGER, SpellFamilyMask2 INTEGER, Cooldown INTEGER, '
                         'SpellFamilyName INTEGER, SpellFamilyMask0 INTEGER, SpellFamilyMask1 INTEGER, '
                         'ProcFlags INTEGER, SpellTypeMask INTEGER, SpellPhaseMask INTEGER, SchoolMask INTEGER)')
        database.executemany('INSERT INTO spell_proc (SpellId, SpellFamilyMask2, Cooldown) VALUES (?, ?, ?)',
                             [(-63373, 0, 0), (63374, 8, 0), (263374, 0, 0), (123, 8, 42)])
        frozen_proc = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000002_area52_frozen_power_proc.sql').read_text()
        database.executescript(frozen_proc)
        database.executescript(frozen_proc)
        assert database.execute('SELECT SpellId, SpellFamilyMask2, Cooldown FROM spell_proc ORDER BY SpellId').fetchall() == [
            (-63373, 536870912, 10000), (123, 8, 42), (63374, 536870920, 10000), (263374, 536870912, 10000)]
        database.execute('INSERT INTO spell_proc (SpellId) VALUES (-51664)')
        database.execute("INSERT INTO spell_script_names VALUES (-51664, 'spell_rog_cut_to_the_chase')")
        cut_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000003_area52_cut_to_the_chase.sql').read_text()
        database.executescript(cut_migration)
        database.executescript(cut_migration)
        assert database.execute('SELECT ScriptName FROM spell_script_names WHERE spell_id=-51664').fetchall() == [
            ('aura_area52_cut_to_the_chase',)]
        assert database.execute('SELECT ProcFlags, SpellTypeMask, SpellPhaseMask FROM spell_proc WHERE SpellId=-51664').fetchone() == (0x15510, 7, 2)
        database.execute('INSERT INTO spell_proc (SpellId) VALUES (979135)')
        database.execute("INSERT INTO spell_script_names VALUES (14177, 'spell_rog_cold_blood')")
        freezing_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000004_area52_freezing_blood.sql').read_text()
        database.executescript(freezing_migration)
        database.executescript(freezing_migration)
        assert database.execute('SELECT ScriptName FROM spell_script_names WHERE spell_id=14177 ORDER BY ScriptName').fetchall() == [
            ('spell_area52_cold_blood_activation',), ('spell_rog_cold_blood',)]
        assert database.execute('SELECT ProcFlags, SchoolMask FROM spell_proc WHERE SpellId=979135').fetchone() == (0x10010, 16)
        assert database.execute('SELECT ScriptName FROM spell_script_names WHERE spell_id=530920').fetchall() == [
            ('aura_area52_freezing_blood_consumed',)]
    effects = (ROOT / 'src/server/game/Spells/SpellEffects.cpp').read_text()
    eviscerate = effects.split('// Eviscerate\n', 1)[1].split('break;', 1)[0]
    assert 'EviscerateScalingBonus(area52, ap,' in eviscerate
    assert 'SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_MAGIC), combo)' in eviscerate
    assert 'damage += int32(ap * combo * 0.07f)' not in eviscerate
    grant_functions = source.split('void RefreshWhiteWalker', 1)[1].split('void RefreshAbsoluteZero', 1)[0]
    absolute_zero = source.split('void RefreshAbsoluteZero', 1)[1].split('class white_walker_player', 1)[0]
    grant_generated = GRANT_MOCKS + '\nvoid RefreshWhiteWalker' + grant_functions + '\nvoid RefreshAbsoluteZero' + absolute_zero + GRANT_CASES
    benediction = source.split('class aura_area52_benediction :', 1)[1].split('void Register()', 1)[0]
    benediction_generated = BENEDICTION_MOCKS + 'class aura_area52_benediction :' + benediction + '};' + BENEDICTION_CASES
    party = source.split('class spell_area52_benediction_party :', 1)[1].split('void Register()', 1)[0]
    benediction_generated = benediction_generated.replace(BENEDICTION_CASES,
        'class spell_area52_benediction_party :' + party + '};' + BENEDICTION_CASES)
    cut = source.split('class aura_area52_cut_to_the_chase :', 1)[1].split('void Register()', 1)[0]
    benediction_generated = benediction_generated.replace(BENEDICTION_CASES,
        'class aura_area52_cut_to_the_chase :' + cut + '};' + BENEDICTION_CASES)
    activation = source.split('class spell_area52_cold_blood_activation :', 1)[1].split('void Register()', 1)[0]
    freezing = source.split('class aura_area52_freezing_blood :', 1)[1].split('void Register()', 1)[0]
    benediction_generated = benediction_generated.replace(BENEDICTION_CASES,
        'class spell_area52_cold_blood_activation :' + activation + '};' +
        'class aura_area52_freezing_blood :' + freezing + '};' + BENEDICTION_CASES)
    consumed = source.split('class aura_area52_freezing_blood_consumed :', 1)[1].split('void Register()', 1)[0]
    bonus = source.split('class aura_area52_freezing_blood_damage :', 1)[1].split('void Register()', 1)[0]
    benediction_generated = benediction_generated.replace(BENEDICTION_CASES,
        'class aura_area52_freezing_blood_consumed :' + consumed + '};' +
        'class aura_area52_freezing_blood_damage :' + bonus + '};' + BENEDICTION_CASES)
    assert 'OnPlayerCanApplyEnchantment' in source
    assert 'FrostbrandEnchantments' in source
    assert 'RemoveAurasDueToItemSpell(helper, item->GetGUID())' in source
    assert 'RemoveAurasDueToItemSpell(otherHelper, item->GetGUID())' in source
    assert 'GetCastItemGUID() == itemGuid' in source
    assert 'HasItemHelper(player, helper, item->GetGUID())' in source
    assert 'AbsoluteZeroReplacement' in source
    assert 'SetTemporarySpellReplacement(rank, player->HasActiveSpell(rank) ? AbsoluteZeroReplacement : 0)' in source
    assert 'AbsoluteZeroGranted.erase(guid)' in source
    assert 'AfterCast += SpellCastFn(spell_area52_cold_blood_activation::Activate)' in source
    assert 'player->CastSpell(target, *rank, TRIGGERED_FULL_MASK)' in source
    assert 'getMSTimeDiff(last->second, now) < 1000' in source
    assert 'ModifySpellCooldown(AscensionWhiteWalkerSupport::ColdBlood, -10000)' in source
    assert 'event.GetSpellInfo()->Id == AscensionWhiteWalkerSupport::SinisterStrike' in source
    assert 'event.GetDamageInfo()->GetDamage()' in source
    assert 'AscensionWhiteWalkerSupport::FightersPower' in source
    assert 'event.GetProcSpell() ? std::max(0, event.GetProcSpell()->GetPowerCost()) : 0' in source
    assert 'preCostEnergy * 100 >= player->GetMaxPower(POWER_ENERGY) * 90' in source
    conditional = (ROOT / 'src/server/coa/AscensionConditionalCombat.cpp').read_text()
    frozen_conversion = conditional.split('void ApplyFrozenPowerDamageContract', 1)[1].split('bool HasExpectedEvilDoersLegacy', 1)[0]
    frozen_generated = FROZEN_POWER_MOCKS + '\nvoid ApplyFrozenPowerDamageContract' + frozen_conversion + FROZEN_POWER_CASES
    assert 'ApplyFrozenPowerDamageContract' in conditional
    assert 'SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE' in conditional
    assert 'ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE' in conditional
    assert 'FrozenPowerTalentPercent' in conditional and 'FrozenPowerMysticPercent' in conditional
    assert '8034, 8037, 10458, 16352, 16353, 965999, 58797, 58798, 58799' in (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
    handler = source.split('class item_area52_frostbrand :', 1)[1].split('class player_area52_frostbrand', 1)[0]
    handler = 'class item_area52_frostbrand :' + handler
    lifecycle = source.split('class player_area52_frostbrand :', 1)[1].split('class aura_area52_slice_and_ice', 1)[0]
    handler += '\nclass player_area52_frostbrand :' + lifecycle
    generated = MAIN.replace('int main()', FROSTBRAND_MOCKS + handler + '\nint main()')
    generated = generated.replace('    static_assert(AthleticCostPercent', FROSTBRAND_CASES + '\n    static_assert(AthleticCostPercent', 1)
    player_source = (ROOT / 'src/server/game/Entities/Player/Player.cpp').read_text()
    enchant_path = player_source.split('// item combat enchantments', 1)[1].split('void Player::CastItemUseSpell', 1)[0]
    assert 'OnCastItemCombatSpell(this, target, spellInfo, item)' in enchant_path
    dispatcher = (ROOT / 'src/server/game/Scripting/ScriptDefines/AllItemScript.cpp').read_text()
    assert 'script->OnItemCombatSpell(player, victim, spellInfo, item)' in dispatcher
    compiler = shutil.which('c++') or shutil.which('g++')
    if not compiler:
        raise SystemExit('UNAVAILABLE: C++ compiler required')
    with tempfile.TemporaryDirectory(prefix='white-walker-support-') as temporary:
        folder = Path(temporary)
        source = folder / 'rules.cpp'
        binary = folder / 'rules'
        source.write_text(generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        source.write_text(grant_generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        source.write_text(benediction_generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        source.write_text(frozen_generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT / 'src/server/coa'), str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print('PASS: White Walker support rules; production Frostbrand handler all ranks, victim/item ownership and exclusions')

if __name__ == '__main__':
    main()
