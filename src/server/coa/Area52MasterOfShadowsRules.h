#ifndef AREA52_MASTER_OF_SHADOWS_RULES_H
#define AREA52_MASTER_OF_SHADOWS_RULES_H
#include <array>
#include <cstddef>
#include <cstdint>

namespace Area52MasterOfShadows
{
constexpr std::uint32_t MasterOfShadows = 980046;
constexpr std::uint32_t ApprenticeOfShadows = 980048;
constexpr std::uint32_t MasterOfShadowsBuff = 980049;
constexpr std::uint32_t MasterOfShadowsCritical = 980065;
constexpr std::uint32_t DarkIntent = 980050;
constexpr std::uint32_t DarkIntentNoCrit = 990248;
constexpr std::uint32_t ShadowTrance = 17941;
constexpr std::uint32_t CrimsonTempest = 254902;
constexpr std::uint32_t Pestilence = 81182;
constexpr std::uint32_t ShadowStrikes = 979750;
constexpr std::uint32_t CruelNight = 1587030;
constexpr std::uint32_t FindWeakness = 931234;
constexpr std::array<std::uint32_t, 6> Gloomblade = {{760050, 760412, 760413, 760414, 760415, 760416}};

constexpr std::uint32_t SpellFamilyWarlock = 5;
constexpr std::uint32_t SpellFamilyRogue = 8;
constexpr std::uint32_t RogueSinisterStrikeFlag0 = 0x2;
constexpr std::uint32_t RogueGarroteFlag0 = 0x100;
constexpr std::uint32_t RogueEviscerateFlag0 = 0x20000;
constexpr std::uint32_t RogueRuptureFlag0 = 0x100000;
constexpr std::uint32_t WarlockShadowBoltFlag0 = 0x1;
constexpr std::uint32_t WarlockCorruptionFlag0 = 0x2;
constexpr std::uint32_t DarkIntentTriggerFlags0 =
    RogueSinisterStrikeFlag0 | RogueGarroteFlag0 | RogueEviscerateFlag0 | RogueRuptureFlag0;
constexpr std::uint32_t ApprenticeSpenderFlags0 = RogueEviscerateFlag0 | RogueRuptureFlag0;

constexpr std::uint8_t ApprenticeStacksForMaster = 3;
constexpr std::uint8_t NightfallDarkIntentHits = 2;
constexpr float PestilenceRadius = 10.0f;
constexpr std::size_t PestilenceMaxTargets = 6;

struct SpellKey
{
    std::uint32_t Id;
    std::uint32_t Family;
    std::uint32_t Flags0;
};

inline bool IsGloomblade(std::uint32_t spell)
{
    for (std::uint32_t id : Gloomblade)
        if (id == spell)
            return true;
    return false;
}

inline bool IsDarkIntent(std::uint32_t spell)
{
    return spell == DarkIntent || spell == DarkIntentNoCrit;
}

inline bool IsSinisterStrike(SpellKey const& spell)
{
    return !IsDarkIntent(spell.Id) && spell.Family == SpellFamilyRogue && (spell.Flags0 & RogueSinisterStrikeFlag0);
}

inline bool TriggersDarkIntent(SpellKey const& spell, bool periodic)
{
    if (IsDarkIntent(spell.Id))
        return false;
    if (IsGloomblade(spell.Id))
        return !periodic;
    if (spell.Id == CrimsonTempest)
        return true;
    return spell.Family == SpellFamilyRogue && (spell.Flags0 & DarkIntentTriggerFlags0);
}

inline bool GrantsApprentice(SpellKey const& spell, bool periodic, bool critical)
{
    return critical && !periodic && (IsSinisterStrike(spell) || IsGloomblade(spell.Id));
}

inline bool SpendsApprentice(SpellKey const& spell, bool periodic, std::uint8_t stacks)
{
    return !periodic && stacks >= ApprenticeStacksForMaster && spell.Family == SpellFamilyRogue &&
        (spell.Flags0 & ApprenticeSpenderFlags0);
}

inline bool IsShadowBolt(SpellKey const& spell)
{
    return spell.Family == SpellFamilyWarlock && (spell.Flags0 & WarlockShadowBoltFlag0);
}

inline bool ShouldCopyRemaining(std::int32_t sourceRemainingMs, std::int32_t targetRemainingMs, bool targetHasIt)
{
    return sourceRemainingMs > 0 && (!targetHasIt || targetRemainingMs < sourceRemainingMs);
}
}
#endif
