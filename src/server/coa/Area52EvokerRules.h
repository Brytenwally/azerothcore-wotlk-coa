#ifndef AREA52_EVOKER_RULES_H
#define AREA52_EVOKER_RULES_H
#include <cstdint>

namespace Area52Evoker
{
constexpr std::uint32_t Evoker = 953308;
constexpr std::uint32_t ArcaneEvoked = 953309;
constexpr std::uint32_t FireEvoked = 953311;
constexpr std::uint32_t FrostEvoked = 953313;
constexpr std::uint32_t EvokerBuff = 953316;
constexpr std::uint32_t MissileBarrage = 44401;
constexpr std::uint32_t BrainFreeze = 57761;
constexpr std::uint32_t BrainFreezeEnchant = 944546;
constexpr std::uint32_t Shatter = 979763;
constexpr std::uint32_t ShatterBuff = 979764;
constexpr std::uint32_t FrostyFrostFrostbolt = 984622;
constexpr std::uint32_t Chills = 984623;
constexpr std::uint32_t FrostyEmpowered = 984625;
constexpr std::uint32_t FrostyDamage = 984624;
constexpr std::uint32_t FrostyProcChance = 984656;
constexpr std::uint32_t ElementBoundMissiles = 783324;
constexpr std::uint32_t ElementBoundMissilesHelper = 783325;
constexpr std::uint32_t ElementBoundHaste = 783100;
constexpr std::uint32_t ElementBoundMana = 783101;
constexpr std::uint32_t WorldforgedCooldown = 780999;
constexpr std::uint32_t CoolDown = 982031;

constexpr std::uint32_t SpellFamilyMage = 3;
constexpr std::uint32_t MageFireballFlag0 = 0x1;
constexpr std::uint32_t MageFrostboltFlag0 = 0x20;
constexpr std::uint32_t MageArcaneMissilesFlag0 = 0x800;
constexpr std::uint32_t MageArcaneMissileHitFlag0 = 0x200000;
constexpr std::uint32_t MageFrostfireBoltFlag1 = 0x1000;
constexpr std::uint32_t BrainFreezeTalentFlag1 = 0x10000000;

constexpr std::uint8_t ChillsForEmpowerment = 2;
constexpr std::int32_t FrostyChanceGraceMs = 1500;

struct SpellKey
{
    std::uint32_t Family;
    std::uint32_t Flags0;
    std::uint32_t Flags1;
};

inline bool IsMage(SpellKey const& spell)
{
    return spell.Family == SpellFamilyMage;
}

inline bool IsFrostbolt(SpellKey const& spell)
{
    return IsMage(spell) && (spell.Flags0 & MageFrostboltFlag0);
}

inline bool IsFireball(SpellKey const& spell)
{
    return IsMage(spell) && (spell.Flags0 & MageFireballFlag0);
}

inline bool IsArcaneMissiles(SpellKey const& spell)
{
    return IsMage(spell) && (spell.Flags0 & MageArcaneMissilesFlag0);
}

inline bool IsFrostfireBolt(SpellKey const& spell)
{
    return IsMage(spell) && (spell.Flags1 & MageFrostfireBoltFlag1);
}

inline bool ClaimsShatter(SpellKey const& spell, bool triggered, bool claimed)
{
    return !triggered && !claimed && IsMage(spell) && !IsFrostbolt(spell);
}

struct ChillStep
{
    std::uint8_t Chills;
    bool Empowers;
};

inline ChillStep AddChill(std::uint8_t chills)
{
    std::uint8_t const next = chills + 1;
    if (next >= ChillsForEmpowerment)
        return {0, true};
    return {next, false};
}

inline std::int32_t FrostyChanceDurationMs(std::uint64_t travelMs)
{
    return std::int32_t(travelMs) + FrostyChanceGraceMs;
}
}
#endif
