#ifndef AREA52_HUNTER_TALENT_RULES_H
#define AREA52_HUNTER_TALENT_RULES_H
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <optional>

namespace Area52HunterTalents
{
constexpr std::uint32_t ImprovedAspectOfTheMonkey[] = {19549, 19550, 19551};
constexpr std::uint32_t FrenzyEffects[] = {19615, 1119615, 1583031};
constexpr std::uint32_t FocusedFire = 35029;
constexpr std::uint32_t KillCommandPetAura = 58914;
constexpr std::uint32_t PetSpecialAbilityFlag1 = 0x10000000;
constexpr std::uint32_t TheBeastWithin = 34692;
constexpr std::uint32_t TheBeastWithinRageChancePct[] = {50, 100};

struct RankGrant
{
    std::uint32_t Talent;
    std::uint32_t Helper;
};

constexpr RankGrant RankGrants[] = {
    {34482, 834482}, {34483, 834483}, {34484, 834484},
    {34948, 984492}, {34949, 984493}, {834949, 984494},
    {53234, 984498}, {53237, 984499}, {53238, 984500}};
constexpr std::uint32_t TrueshotAura = 19506;
constexpr std::uint32_t ImprovedSteadyShotCapstone = 53224;
constexpr std::uint32_t CapstoneSteadyShotBuff = 853220;
constexpr std::uint32_t CapstoneArmorPierce = 853221;
constexpr std::uint32_t SteadyShotFlag1 = 0x1;
constexpr std::uint32_t ImprovedStingsCapstone = 19466;
constexpr std::uint32_t SerpentsKiss = 851978;
constexpr std::uint32_t SerpentsKissDamage = 851979;
constexpr std::uint32_t SerpentSting = 1978;
constexpr std::uint32_t ChimeraShotFlag2 = 0x1;
constexpr std::uint32_t MortalShots[] = {19485, 19487, 19488};
constexpr std::uint32_t AimedShotFlag0 = 0x20000;

template <std::size_t N>
constexpr bool Contains(std::uint32_t const (&ids)[N], std::uint32_t id)
{
    for (std::uint32_t candidate : ids)
        if (candidate == id)
            return true;
    return false;
}

inline std::optional<bool> ModifierTarget(std::uint32_t modifier, std::uint32_t target)
{
    if (Contains(ImprovedAspectOfTheMonkey, modifier) && Contains(FrenzyEffects, target))
        return false;
    return std::nullopt;
}

inline std::uint32_t RankGrantHelper(std::uint32_t talent)
{
    for (RankGrant const& grant : RankGrants)
        if (grant.Talent == talent)
            return grant.Helper;
    return 0;
}

inline std::int32_t MortalShotsAimedShotDurationMs(std::uint32_t talent)
{
    for (std::size_t rank = 0; rank < std::size(MortalShots); ++rank)
        if (MortalShots[rank] == talent)
            return std::int32_t(rank + 1) * 1000;
    return 0;
}

inline bool SteadyShotCapstoneApplies(bool hasCapstone, bool moving)
{
    return hasCapstone && !moving;
}

inline std::uint32_t BeastWithinRageChance(std::uint8_t rank)
{
    return rank >= 2 ? TheBeastWithinRageChancePct[1] : TheBeastWithinRageChancePct[0];
}

inline bool FocusedFireApplies(std::uint32_t familyFlag1, bool killCommandActive)
{
    return killCommandActive && (familyFlag1 & PetSpecialAbilityFlag1);
}
}

#endif
