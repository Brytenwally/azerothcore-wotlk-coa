import argparse
from pathlib import Path
import shutil
import sqlite3
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[3]

COST_MOCKS = r'''
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <set>
#include <vector>
using int32 = std::int32_t;
using uint32 = std::uint32_t;
constexpr int EFFECT_0 = 0;
constexpr int SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT = 72;
struct AuraEffect
{
    uint32 id;
    int32 amount;
    uint32 schools;
    int type = SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT;
    int32 GetAmount() const { return amount; }
    uint32 GetMiscValue() const { return schools; }
    uint32 GetId() const { return id; }
    int GetAuraType() const { return type; }
};
struct Unit
{
    std::vector<AuraEffect const*> effects;
    AuraEffect const* GetAuraEffect(uint32 id, int) const
    {
        for (auto effect : effects) if (effect->id == id) return effect;
        return nullptr;
    }
    auto const& GetAuraEffectsByType(int) const { return effects; }
};
bool Ready = true;
struct Catalog
{
    std::set<uint32> spells{84561, 911160, 1329440};
    bool FindSpell(uint32 id) const { return spells.contains(id); }
} Loaded;
'''

COST_CASES = r'''
int main()
{
    AuraEffect athletic{84561, -4, 17};
    AuraEffect channeling{911160, -3, 84};
    AuraEffect talent{111, -5, 16};
    AuraEffect similar{1329440, -3, 126};
    Unit caster{{&athletic, &channeling, &talent}};
    for (int copies = 1; copies <= 3; ++copies)
    {
        channeling.amount = -3 * copies;
        float correction = NonstackingCostCorrection(&caster, 16);
        assert(std::abs(correction - std::min(4, 3 * copies) / 100.0f) < .00001f);
        float combined = (athletic.amount + channeling.amount + talent.amount) / 100.0f + correction;
        assert(std::abs(combined - (-std::max(4, 3 * copies) - 5) / 100.0f) < .00001f);
        assert(NonstackingCostCorrection(&caster, 1) == 0);
        assert(NonstackingCostCorrection(&caster, 4) == 0);
        assert(NonstackingCostCorrection(&caster, 64) == 0);
    }
    caster.effects.push_back(&similar);
    assert(std::abs(NonstackingCostCorrection(&caster, 16) - .07f) < .00001f);
    caster.effects = {&athletic, &talent};
    assert(NonstackingCostCorrection(&caster, 16) == 0);
    caster.effects = {&channeling, &talent};
    assert(NonstackingCostCorrection(&caster, 16) == 0);
    caster.effects = {&athletic, &channeling};
    Ready = false;
    assert(NonstackingCostCorrection(&caster, 16) == 0);
    Ready = true;
    athletic.type = 4;
    assert(NonstackingCostCorrection(&caster, 16) == 0);
    athletic.type = SPELL_AURA_MOD_POWER_COST_SCHOOL_PCT;
    athletic.amount = 4;
    assert(NonstackingCostCorrection(&caster, 16) == 0);
    athletic.amount = -4;
    channeling.amount = 3;
    assert(NonstackingCostCorrection(&caster, 16) == 0);
}
'''

DAMAGE_PREDICATE_MOCKS = r'''
#include <cassert>
#include <set>
#include <cstdint>
using uint32 = std::uint32_t;
using AuraStateType = int;
constexpr int ASCENSION_CLASSMASK_AURASTATE_DAMAGE = 20007;
constexpr int ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE = 20012;
constexpr int ASCENSION_CLASSMASK_NPC_DAMAGE = 20013;
constexpr int ASCENSION_CLASSMASK_PLAYER_DAMAGE = 20014;
constexpr int SPELL_SCHOOL_MASK_FROST = 16;
constexpr int SPELL_SCHOOL_MASK_FIRE = 4;
constexpr int SPELL_SCHOOL_MASK_NATURE = 8;
constexpr int DOT = 1;
struct Unit
{
    bool player = false;
    bool state = false;
    std::set<uint32> auras;
    bool IsPlayer() const { return player; }
    bool HasAura(uint32 id) const { return auras.contains(id); }
    bool HasAuraState(int) const { return state; }
};
struct AuraEffect;
struct SpellInfo
{
    uint32 school = 16;
    bool attribute = true;
    uint32 GetSchoolMask() const { return school; }
    bool ValidateAttribute6SpellDamageMods(Unit const*, AuraEffect const*, bool) const { return attribute; }
};
struct AuraEffect
{
    int marker = ASCENSION_CLASSMASK_NPC_DAMAGE;
    bool affected = true;
    int GetMiscValueB() const { return marker; }
    int GetMiscValue() const { return 0; }
    bool IsAffectedOnSpell(SpellInfo const*) const { return affected; }
};
'''

DAMAGE_PREDICATE_CASES = r'''
int main()
{
    Unit target, caster;
    SpellInfo spell;
    AuraEffect effect;
    auto check = [&]
    {
        assert(SpellPredicate(&target, &spell, &caster, 0)(&effect) ==
            MeleePredicate(&target, &spell, &caster)(&effect));
        return SpellPredicate(&target, &spell, &caster, 0)(&effect);
    };
    assert(check());
    target.player = true;
    assert(!check());
    effect.marker = ASCENSION_CLASSMASK_PLAYER_DAMAGE;
    assert(check());
    target.player = false;
    assert(!check());
    target.player = true;
    effect.affected = false;
    assert(!check());
    effect.affected = true;
    spell.attribute = false;
    assert(!check());
    spell.attribute = true;
    assert(!SpellPredicate(&target, nullptr, &caster, 0)(&effect));
    assert(!MeleePredicate(&target, nullptr, &caster)(&effect));
    effect.marker = ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE;
    assert(!check());
    for (uint32 slow : {8034u, 8037u, 10458u, 16352u, 16353u, 965999u, 58797u, 58798u, 58799u})
    {
        target.auras = {slow};
        for (uint32 school : {4u, 8u, 16u})
        {
            spell.school = school;
            assert(check());
            assert(SpellPredicate(&target, &spell, &caster, DOT)(&effect));
        }
        for (uint32 school : {1u, 2u, 32u, 64u})
        {
            spell.school = school;
            assert(!check());
        }
    }
    spell.school = 16;
    spell.attribute = false;
    assert(!check());
    spell.attribute = true;
    target.auras.clear();
    assert(!check());
}
'''

PROC_RANK_MOCKS = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
using uint32 = std::uint32_t;
using uint8 = std::uint8_t;
using uint64 = std::uint64_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
bool Area52Enabled = true;
constexpr uint32 PROC_HIT_CRITICAL = 2;
struct Unit
{
    bool twoHanded = true;
    bool slice = true;
    bool fury = false;
    bool worldforgedCooldown = false;
    uint32 level = 80;
    bool tides = true;
    int32 energy = 100;
    uint32 maximumEnergy = 100;
    int critModifier = 0;
    std::vector<uint32> casts;
    std::vector<uint32> removed;
    std::vector<std::array<int32, 3>> custom;
    Unit* ToPlayer() { return this; }
    bool IsPlayer() const { return true; }
    bool IsTwoHandUsed() const { return twoHanded; }
    bool HasAura(uint32 id) const
    {
        return (id == AscensionWhiteWalkerSupport::SliceAndIce && slice) ||
            (id == AscensionWhiteWalkerSupport::WintersFury && fury) || (id == 780999 && worldforgedCooldown) ||
            (id == AscensionWhiteWalkerSupport::TidesOfBattle && tides);
    }
    void CastSpell(Unit*, uint32 id, int) { casts.push_back(id); }
    uint32 GetGUID() const { return 1; }
    uint32 GetLevel() const { return level; }
    uint32 GetPower(int) const { return energy; }
    uint32 GetMaxPower(int) const { return maximumEnergy; }
    void ModifyPower(int, int32 change) { energy = std::clamp<int32>(energy + change, 0, maximumEnergy); }
    void CastCustomSpell(Unit*, uint32 id, int32* first, int32* second, int32*, bool)
    {
        custom.push_back({static_cast<int32>(id), first ? *first : 0, second ? *second : 0});
    }
    void RemoveAurasDueToSpell(uint32 id, uint32 guid = 1)
    {
        assert(guid == 1);
        removed.push_back(id);
        if (id == AscensionWhiteWalkerSupport::WintersFury) fury = false;
    }
    int GetTotalAuraModifier(int) const { return critModifier; }
};
using Player = Unit;
struct SpellInfo
{
    uint32 Id;
    bool cantCrit = false;
    bool IsCritCapable() const { return true; }
    bool HasAttribute(int) const { return cantCrit; }
};
struct Spell
{
    bool triggered = false;
    Unit* caster = nullptr;
    SpellInfo* info = nullptr;
    std::map<uint32, uint64> values{};
    bool IsTriggered() const { return triggered; }
    Unit* GetCaster() const { return caster; }
    SpellInfo const* GetSpellInfo() const { return info; }
    uint64 GetScriptValue(uint32 id) const { auto found = values.find(id); return found == values.end() ? 0 : found->second; }
    void SetScriptValue(uint32 id, uint64 value) { values[id] = value; }
};
constexpr int SPELL_MISS_NONE = 0;
constexpr int SPELL_ATTR2_CANT_CRIT = 1;
constexpr int SPELL_AURA_MOD_ATTACKER_SPELL_AND_WEAPON_CRIT_CHANCE = 2;
struct TargetInfo { int damage = 100; int missCondition = SPELL_MISS_NONE; bool crit = false; };
struct DamageInfo { uint32 amount = 100; uint32 GetDamage() const { return amount; } };
struct ProcEventInfo
{
    Unit* actor;
    SpellInfo* spell;
    DamageInfo* damage;
    uint32 hit = PROC_HIT_CRITICAL;
    Spell cast{};
    Spell const* GetProcSpell() const { return &cast; }
    Unit* GetActor() const { return actor; }
    SpellInfo const* GetSpellInfo() const { return spell; }
    DamageInfo const* GetDamageInfo() const { return damage; }
    uint32 GetHitMask() const { return hit; }
};
struct SpellMgr
{
    std::map<uint32, uint32> roots;
    uint32 GetFirstSpellInChain(uint32 id) const
    {
        auto found = roots.find(id);
        return found == roots.end() ? id : found->second;
    }
};
SpellMgr manager;
SpellMgr* sSpellMgr = &manager;
Unit owner, other;
Unit* GetTarget() { return &owner; }
constexpr int TRIGGERED_FULL_MASK = 1;
bool prevented = false;
void PreventDefaultAction() { prevented = true; }
struct AuraEffect { int32 amount = -3; int32 GetAmount() const { return amount; } };
AuraEffect drainEffect;
AuraEffect const* GetEffect(int) { return &drainEffect; }
constexpr int POWER_ENERGY = 3;
constexpr int EFFECT_1 = 1;
using AuraEffectHandleModes = int;
uint8 enchantCopies = 1;
uint8 GetStackAmount() { return enchantCopies; }
'''

PROC_RANK_CASES = r'''
int main()
{
    std::array<uint32, 12> sinister{1752, 1757, 1758, 1759, 1760, 8621, 11293, 11294,
        26861, 26862, 48637, 48638};
    std::array<uint32, 12> eviscerate{2098, 6760, 6761, 6762, 8623, 8624, 11299, 11300,
        31016, 26865, 48667, 48668};
    for (uint32 rank : sinister) manager.roots[rank] = 1752;
    for (uint32 rank : eviscerate) manager.roots[rank] = 2098;
    for (uint32 rank : AscensionWhiteWalkerSupport::IceLanceRanks) manager.roots[rank] = 30455;
    DamageInfo damage;
    SpellInfo spell{1752};
    ProcEventInfo event{&owner, &spell, &damage};
    event.cast.caster = &owner;
    event.cast.info = &spell;
    SnapshotTides(&owner, &event.cast, false);
    for (uint32 rank : sinister)
    {
        spell.Id = rank;
        assert(WhiteWalkerProc(event) && FighterProc(event) && TidesProc(event));
        assert(!SliceProc(event));
    }
    for (uint32 rank : eviscerate)
    {
        spell.Id = rank;
        assert(WhiteWalkerProc(event));
        assert(!FighterProc(event) && !TidesProc(event) && !SliceProc(event));
    }
    for (uint32 rank : AscensionWhiteWalkerSupport::IceLanceRanks)
    {
        spell.Id = rank;
        assert(SliceProc(event));
        assert(!WhiteWalkerProc(event) && !FighterProc(event) && !TidesProc(event));
    }
    for (auto check : {WhiteWalkerProc, FighterProc, TidesProc, SliceProc})
    {
        spell.Id = check == SliceProc ? 42914 : 48638;
        assert(check(event));
        event.actor = &other;
        assert(!check(event));
        event.actor = &owner;
        damage.amount = 0;
        assert(!check(event));
        damage.amount = 100;
        event.damage = nullptr;
        assert(!check(event));
        event.damage = &damage;
        event.spell = nullptr;
        assert(!check(event));
        event.spell = &spell;
        Area52Enabled = false;
        assert(!check(event));
        Area52Enabled = true;
    }
    spell.Id = 48638;
    owner.slice = false;
    assert(!WhiteWalkerProc(event));
    owner.slice = true;
    owner.twoHanded = false;
    assert(!WhiteWalkerProc(event));
    owner.twoHanded = true;
    event.hit = 0;
    assert(!WhiteWalkerProc(event));
    spell.Id = 42914;
    assert(!SliceProc(event));
    event.hit = PROC_HIT_CRITICAL;
    event.cast.triggered = true;
    assert(!SliceProc(event));
    SliceDispatch(event);
    assert(owner.casts == std::vector<uint32>{AscensionWhiteWalkerSupport::WintersVitality});
    event.cast.triggered = false;
    damage.amount = 0;
    assert(AvailabilityProc(event));
    event.cast.triggered = true;
    assert(!AvailabilityProc(event));
    event.cast.triggered = false;
    event.actor = &other;
    assert(!AvailabilityProc(event));
    event.actor = &owner;
    owner.slice = false;
    assert(!AvailabilityProc(event));
    owner.slice = true;
    owner.twoHanded = false;
    assert(!AvailabilityProc(event));
    owner.twoHanded = true;
    owner.casts.clear();
    ApplyAvailability(nullptr, 0);
    assert(owner.casts == std::vector<uint32>{984937});
    AvailabilityDispatch(nullptr, event);
    assert((owner.casts == std::vector<uint32>{984937, AscensionWhiteWalkerSupport::WintersFury}));
    RemoveAvailability(nullptr, 0);
    assert(owner.removed == std::vector<uint32>{984937});
    IceSpearCallbacks spear;
    for (uint32 rank : AscensionWhiteWalkerSupport::IceSpearRanks)
    {
        SpellInfo info{rank};
        Spell cast;
        cast.caster = &owner;
        cast.info = &info;
        owner.fury = true;
        owner.casts.clear();
        spear.OnSpellBeforeEffects(&cast, &owner, &info);
        assert(!owner.fury && cast.GetScriptValue(AscensionWhiteWalkerSupport::WintersFury) == 1);
        owner.fury = true;
        spear.OnSpellBeforeEffects(&cast, &owner, &info);
        assert(owner.fury);
        TargetInfo hit;
        spear.OnSpellCalculatedTarget(&cast, &other, hit);
        assert(hit.crit);
        hit.crit = false;
        other.critModifier = -100;
        spear.OnSpellCalculatedTarget(&cast, &other, hit);
        assert(!hit.crit);
        other.critModifier = 0;
        info.cantCrit = true;
        spear.OnSpellCalculatedTarget(&cast, &other, hit);
        assert(!hit.crit);
        info.cantCrit = false;
        hit.missCondition = 1;
        spear.OnSpellCalculatedTarget(&cast, &other, hit);
        assert(!hit.crit);
        spear.OnSpellHitResult(&cast, &other, 1, 100, 0, false);
        assert(owner.casts.empty());
        spear.OnSpellHitResult(&cast, &other, 0, 100, 0, true);
        spear.OnSpellHitResult(&cast, &other, 0, 100, 0, true);
        assert(owner.casts == std::vector<uint32>{979053});
    }
    SpellInfo ordinary{30455};
    Spell ordinaryCast;
    ordinaryCast.caster = &owner;
    ordinaryCast.info = &ordinary;
    owner.fury = true;
    spear.OnSpellBeforeEffects(&ordinaryCast, &owner, &ordinary);
    assert(owner.fury && !ordinaryCast.GetScriptValue(AscensionWhiteWalkerSupport::WintersFury));
    spell.Id = 48638;
    damage.amount = 100;
    event.actor = &owner;
    owner.worldforgedCooldown = true;
    assert(!FighterProc(event));
    owner.worldforgedCooldown = false;
    assert(FighterProc(event));
    for (uint32 level : {1, 40, 80})
        for (enchantCopies = 1; enchantCopies <= 3; ++enchantCopies)
        {
            owner.level = level;
            owner.custom.clear();
            FighterDispatch(event);
            int32 attackPower = level * enchantCopies;
            assert((owner.custom == std::vector<std::array<int32, 3>>{
                {781000, attackPower, attackPower}, {781001, 10 * enchantCopies, 0},
                {781002, 100 * enchantCopies, 0}}));
        }
    for (int32 energy : {0, 89, 90, 91, 100})
    {
        event.cast.values.clear();
        owner.energy = energy;
        owner.maximumEnergy = 100;
        SnapshotTides(&owner, &event.cast, false);
        owner.energy = energy > 50 ? energy - 50 : 0;
        owner.casts.clear();
        owner.removed.clear();
        assert(TidesProc(event));
        TidesDispatch(event);
        assert(owner.casts == std::vector<uint32>{energy >= 90 ? AscensionWhiteWalkerSupport::Ebb :
            AscensionWhiteWalkerSupport::Flow});
        assert(owner.removed == std::vector<uint32>{energy >= 90 ? AscensionWhiteWalkerSupport::Flow :
            AscensionWhiteWalkerSupport::Ebb});
        assert(!TidesProc(event));
    }
    event.cast.values.clear();
    event.cast.triggered = true;
    SnapshotTides(&owner, &event.cast, false);
    assert(!TidesProc(event));
    event.cast.triggered = false;
    owner.tides = false;
    SnapshotTides(&owner, &event.cast, false);
    assert(!TidesProc(event));
    owner.tides = true;
    for (uint32 maximum : {100, 200})
        for (int copies = 1; copies <= 5; ++copies)
        {
            owner.maximumEnergy = maximum;
            owner.energy = maximum;
            drainEffect.amount = -3 * copies;
            prevented = false;
            EbbTick(nullptr);
            assert(prevented && owner.energy == int32(maximum - maximum * 3 * copies / 100));
            owner.energy = 1;
            EbbTick(nullptr);
            assert(owner.energy == 0);
        }
    drainEffect.amount = 3;
    owner.energy = 50;
    EbbTick(nullptr);
    assert(owner.energy == 50);
}
'''

FROZEN_POWER_MOCKS = r'''
#include "AscensionWhiteWalkerSupportRules.h"
#include <array>
#include <cassert>
using uint32 = std::uint32_t;
using uint8 = std::uint8_t;
constexpr int EFFECT_0 = 0;
constexpr int EFFECT_1 = 1;
constexpr int SPELL_EFFECT_APPLY_AURA = 6;
constexpr int SPELL_AURA_ADD_FLAT_MODIFIER = 107;
constexpr int SPELL_AURA_ADD_PCT_MODIFIER = 108;
constexpr int SPELL_AURA_MOD_RATING_FROM_STAT = 220;
constexpr int SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE = 303;
constexpr int ASCENSION_CLASSMASK_FROSTBRAND_DAMAGE = 20012;
constexpr int ASCENSION_CLASSMASK_NPC_DAMAGE = 20013;
constexpr int ASCENSION_CLASSMASK_PLAYER_DAMAGE = 20014;
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
struct SpellInfo { uint32 Id; std::array<SpellEffectInfo, 3> Effects; uint32 SpellFamilyName = 8; };
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
    SpellInfo helper{878302, {}};
    helper.Effects[0].ApplyAuraName = SPELL_AURA_MOD_RATING_FROM_STAT;
    helper.Effects[0].BasePoints = 49;
    helper.Effects[0].MiscValue = 256;
    helper.Effects[0].MiscValueB = 3;
    SpellInfo slice{AscensionWhiteWalkerSupport::SliceAndIce, {}};
    ApplySliceAndIceStats(&slice, &helper);
    assert(slice.Effects[1].Effect == SPELL_EFFECT_APPLY_AURA);
    assert(slice.Effects[1].ApplyAuraName == SPELL_AURA_MOD_RATING_FROM_STAT);
    assert(slice.Effects[1].BasePoints + slice.Effects[1].DieSides == 50);
    assert(slice.Effects[1].MiscValue == 256 && slice.Effects[1].MiscValueB == 3);
    ApplySliceAndIceStats(&unrelated, &helper);
    assert(unrelated.Effects[1].BasePoints == 99);
    helper.Effects[0].ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
    slice.Effects[1].BasePoints = 99;
    ApplySliceAndIceStats(&slice, &helper);
    assert(slice.Effects[1].BasePoints == 99);
    ApplySliceAndIceStats(nullptr, &helper);
    ApplySliceAndIceStats(&slice, nullptr);
    for (uint32 identity : {979053u, 979011u, 530921u})
    {
        SpellInfo info{identity, {}};
        info.SpellFamilyName = identity == 979053 ? 3 : 8;
        std::array<int, 2> values = identity == 979011 ? std::array<int, 2>{125, 25} :
            identity == 530921 ? std::array<int, 2>{10, 20} : std::array<int, 2>{50, 0};
        for (int index = 0; index < (identity == 979053 ? 1 : 2); ++index)
        {
            info.Effects[index].ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
            info.Effects[index].MiscValue = (identity == 530921 ? index == 0 : index == 1) ? 36 : 38;
            info.Effects[index].BasePoints = values[index] - 1;
        }
        ApplyArea52TargetDamageContracts(&info);
        for (int index = 0; index < (identity == 979053 ? 1 : 2); ++index)
        {
            auto const& effect = info.Effects[index];
            assert(effect.ApplyAuraName == SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE);
            assert(effect.BasePoints + effect.DieSides == values[index]);
            assert(effect.MiscValueB == (effect.MiscValue == 36 ? ASCENSION_CLASSMASK_PLAYER_DAMAGE :
                ASCENSION_CLASSMASK_NPC_DAMAGE));
            assert((effect.SpellClassMask == flag96(16777216, 0, 0)));
        }
        ApplyArea52TargetDamageContracts(&info);
    }
    SpellInfo wrongFamily{979053, {}};
    wrongFamily.Effects[0].ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
    wrongFamily.Effects[0].MiscValue = 38;
    ApplyArea52TargetDamageContracts(&wrongFamily);
    assert(wrongFamily.Effects[0].ApplyAuraName == SPELL_AURA_ADD_PCT_MODIFIER);
    ApplyArea52TargetDamageContracts(nullptr);
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
struct SpellInfo { uint32 SpellLevel; };
struct SpellMgr
{
    std::array<SpellInfo, 7> ranks{{{1}, {30}, {40}, {50}, {60}, {70}, {80}}};
    SpellInfo const* GetSpellInfo(uint32 id) const
    {
        auto const& ids = AscensionWhiteWalkerSupport::IceSpearRanks;
        auto found = std::find(ids.begin(), ids.end(), id);
        return found == ids.end() ? nullptr : &ranks[found - ids.begin()];
    }
};
SpellMgr manager;
SpellMgr* sSpellMgr = &manager;
struct Player
{
    bool active = true;
    bool twoHanded = true;
    bool fury = false;
    uint32 level = 1;
    std::map<uint32, bool> spells;
    std::map<uint32, uint32> replacements;
    ObjectGuid GetGUID() const { return {}; }
    bool HasAura(uint32 id) const { return id == AscensionWhiteWalkerSupport::WintersFury ? fury : active; }
    uint32 GetLevel() const { return level; }
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
    for (uint32 root : IceLanceRanks)
    {
        player = Player{};
        WhiteWalkerGranted.clear();
        player.spells[root] = true;
        for (std::size_t rank = 0; rank < IceSpearRanks.size(); ++rank)
        {
            player.level = manager.ranks[rank].SpellLevel;
            player.fury = true;
            RefreshWhiteWalker(&player);
            assert(player.HasActiveSpell(IceSpearRanks[rank]));
            assert(player.replacements[root] == IceSpearRanks[rank]);
            if (rank) assert(!player.HasActiveSpell(IceSpearRanks[rank - 1]));
            player.fury = false;
            RefreshWhiteWalker(&player);
            assert(player.replacements[root] == 0);
        }
        player.twoHanded = false;
        RefreshWhiteWalker(&player);
        assert(!player.HasActiveSpell(IceSpearRanks.back()));
        assert(player.replacements[root] == 0);
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
    args = parser.parse_args()
    if args.dbc_dir:
        sys.path.insert(0, str(ROOT / 'apps/coa-dbc'))
        from inspect_dbc import DataSet
        data = DataSet(Path(args.dbc_dir))
        def fields(table, identity):
            return {field['name']: field['value'] for field in data.record(table, identity)['fields']}
        assert fields('Spell', 781047)['ProcChance'] == 50
        assert fields('Spell', 984937)['ProcChance'] == 20
        assert fields('Spell', 781046)['StackAmount'] == 3
        buff = fields('Spell', 781000)
        assert buff['EffectApplyAuraName[0]'] == 344 and buff['EffectApplyAuraName[1]'] == 124
        assert buff['Effect[2]'] == 64 and buff['EffectTriggerSpell[2]'] == 780999
        assert fields('SpellDuration', buff['DurationIndex'])['Duration'] == 12000
        cooldown = fields('Spell', 780999)
        assert fields('SpellDuration', cooldown['DurationIndex'])['Duration'] == 20000
        for identity, power in [(781001, 3), (781002, 1)]:
            resource = fields('Spell', identity)
            assert resource['Effect[0]'] == 30 and resource['EffectMiscValue[0]'] == power
        for identity, level in zip((984935, 984970, 984971, 984972, 984973, 984974, 984975),
                                   (1, 30, 40, 50, 60, 70, 80)):
            rank = fields('Spell', identity)
            assert rank['SpellLevel'] == level and rank['CasterAuraSpell'] == 984936
        ebb = fields('Spell', 978746)
        assert ebb['StackAmount'] == 5 and ebb['EffectApplyAuraName[2]'] == 23
        assert ebb['EffectAmplitude[2]'] == 1000
        assert ebb['EffectBasePoints[1]'] + ebb['EffectDieSides[1]'] == -3
        assert fields('Spell', 978747)['StackAmount'] == 5
        channeling = fields('Spell', 911160)
        assert channeling['EffectApplyAuraName[0]'] == 72
        assert channeling['EffectBasePoints[0]'] + channeling['EffectDieSides[0]'] == -3
        assert channeling['EffectMiscValue[0]'] == 84 and channeling['StackAmount'] == 3
    source = (ROOT / 'src/server/coa/AscensionWhiteWalkerSupport.cpp').read_text()
    unit_source = (ROOT / 'src/server/game/Entities/Unit/Unit.cpp').read_text()
    predicates = unit_source.split('DoneTotalMod *= GetTotalAuraMultiplier(SPELL_AURA_MOD_DAMAGE_DONE_VERSUS_AURASTATE, ', 2)[1:]
    damage_generated = DAMAGE_PREDICATE_MOCKS
    for name, body, arguments in [('SpellPredicate', predicates[0], ', int damagetype'),
                                  ('MeleePredicate', predicates[1], '')]:
        expression = body.split('});', 1)[0] + '}'
        damage_generated += ('\nauto ' + name + '(Unit const* victim, SpellInfo const* spellProto, '
                             'Unit const* caster' + arguments + ') { return ' + expression.replace('this', 'caster') + '; }\n')
    damage_generated += DAMAGE_PREDICATE_CASES
    mystic_source = (ROOT / 'src/server/coa/AscensionMysticEnchant.cpp').read_text()
    correction = mystic_source.split('float NonstackingCostCorrection', 1)[1].split('bool ApplyGuide', 1)[0]
    cost_generated = COST_MOCKS + '\nfloat NonstackingCostCorrection' + correction + COST_CASES
    assert 'NonstackingCostCorrection(caster, uint32(1) << school)' in (
        ROOT / 'src/server/game/Spells/SpellInfo.cpp').read_text()
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
                         'ProcFlags INTEGER, SpellTypeMask INTEGER, SpellPhaseMask INTEGER, SchoolMask INTEGER, '
                         'HitMask INTEGER, AttributesMask INTEGER, DisableEffectsMask INTEGER, '
                         'ProcsPerMinute REAL, Chance REAL, Charges INTEGER)')
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
        database.execute('INSERT INTO spell_proc (SpellId, Cooldown) VALUES (978302, 1234)')
        slice_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000005_area52_slice_and_ice_proc.sql').read_text()
        database.executescript(slice_migration)
        database.executescript(slice_migration)
        assert database.execute('SELECT ProcFlags, SchoolMask, SpellFamilyMask0, SpellTypeMask, '
                                'SpellPhaseMask, Cooldown FROM spell_proc WHERE SpellId=978302').fetchone() == (
                                    65536, 16, 0, 1, 2, 1234)
        availability_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000006_area52_ice_spear_availability.sql').read_text()
        database.executescript(availability_migration)
        database.executescript(availability_migration)
        assert database.execute('SELECT ProcFlags, SpellTypeMask, SpellPhaseMask, Chance, Cooldown '
                                'FROM spell_proc WHERE SpellId=984937').fetchone() == (0x15510, 7, 1, 0, 0)
        assert database.execute('SELECT ScriptName FROM spell_script_names WHERE spell_id=984937').fetchall() == [
            ('aura_area52_slice_and_ice_availability',)]
        database.execute('INSERT INTO spell_proc (SpellId, Chance, Cooldown) VALUES (781046, 100, 4321)')
        fighter_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000007_area52_fighters_proc.sql').read_text()
        database.executescript(fighter_migration)
        database.executescript(fighter_migration)
        assert database.execute('SELECT ProcFlags, Chance, Cooldown FROM spell_proc WHERE SpellId=781046').fetchone() == (
            16, 0, 4321)
        ebb_migration = (ROOT / 'data/sql/updates/pending_db_world/rev_20261014000008_area52_ebb_drain.sql').read_text()
        database.executescript(ebb_migration)
        database.executescript(ebb_migration)
        assert database.execute('SELECT ScriptName FROM spell_script_names WHERE spell_id=978746').fetchall() == [
            ('aura_area52_ebb_drain',)]
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
    proc_generated = PROC_RANK_MOCKS
    snapshot = source.split('void OnPlayerSpellCast', 1)[1].split('void OnPlayerUpdate', 1)[0]
    proc_generated += '\nvoid SnapshotTides' + snapshot.replace(' override', '')
    for script, function in [('aura_area52_white_walker', 'WhiteWalkerProc'),
                             ('aura_area52_fighters_sinister_strike', 'FighterProc'),
                             ('aura_area52_tides_of_battle', 'TidesProc'),
                             ('aura_area52_slice_and_ice_availability', 'AvailabilityProc'),
                             ('aura_area52_slice_and_ice', 'SliceProc')]:
        body = source.split('class ' + script + ' :', 1)[1].split('bool CheckProc', 1)[1].split('void Proc', 1)[0]
        proc_generated += '\nbool ' + function + body
    slice_dispatch = source.split('class aura_area52_slice_and_ice :', 1)[1].split('void Proc', 1)[1].split('void Register()', 1)[0]
    proc_generated += '\nvoid SliceDispatch' + slice_dispatch.replace('ProcEventInfo& event', 'ProcEventInfo&')
    availability = source.split('class aura_area52_slice_and_ice_availability :', 1)[1]
    proc_generated += '\nvoid AvailabilityDispatch' + availability.split('void Proc', 1)[1].split('void Register()', 1)[0]
    slice_body = source.split('class aura_area52_slice_and_ice :', 1)[1]
    proc_generated += '\nvoid ApplyAvailability' + slice_body.split('void Apply', 1)[1].split('void Remove', 1)[0]
    proc_generated += '\nvoid RemoveAvailability' + slice_body.split('void Remove', 1)[1].split('bool CheckProc', 1)[0]
    spear = source.split('class white_walker_ice_spear :', 1)[1].split('void OnSpellBeforeEffects', 1)[1].split('\n};', 1)[0]
    proc_generated += '\nclass IceSpearCallbacks { public: void OnSpellBeforeEffects' + spear.replace(' override', '') + '\n};\n'
    fighter_dispatch = source.split('class aura_area52_fighters_sinister_strike :', 1)[1].split('void Proc', 1)[1].split('void Register()', 1)[0]
    proc_generated += '\nvoid FighterDispatch' + fighter_dispatch.replace('ProcEventInfo& event', 'ProcEventInfo&')
    tides_dispatch = source.split('class aura_area52_tides_of_battle :', 1)[1].split('void Proc', 1)[1].split('void Register()', 1)[0]
    proc_generated += '\nvoid TidesDispatch' + tides_dispatch
    ebb_tick = source.split('class aura_area52_ebb_drain :', 1)[1].split('void Tick', 1)[1].split('void Register()', 1)[0]
    proc_generated += '\nvoid EbbTick' + ebb_tick
    proc_generated += PROC_RANK_CASES
    assert 'event.GetDamageInfo()->GetDamage()' in source
    assert 'AscensionWhiteWalkerSupport::FightersPower' in source
    assert 'preCostEnergy * 100 >= maximum * 90' in source
    conditional = (ROOT / 'src/server/coa/AscensionConditionalCombat.cpp').read_text()
    frozen_conversion = conditional.split('void ApplyFrozenPowerDamageContract', 1)[1].split('bool HasExpectedEvilDoersLegacy', 1)[0]
    slice_stats = source.split('void ApplySliceAndIceStats', 1)[1].split('void RefreshWhiteWalker', 1)[0]
    frozen_generated = (FROZEN_POWER_MOCKS + '\nvoid ApplySliceAndIceStats' + slice_stats +
                        '\nvoid ApplyFrozenPowerDamageContract' + frozen_conversion + FROZEN_POWER_CASES)
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
        source.write_text(cost_generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        source.write_text(damage_generated)
        subprocess.run([compiler, '-std=c++20', '-Wall', '-Wextra', '-Werror',
                        str(source), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
        source.write_text(proc_generated)
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
    print('PASS: White Walker production handlers, native data contracts, ownership, resource and cost regressions')

if __name__ == '__main__':
    main()
