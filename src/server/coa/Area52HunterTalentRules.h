#ifndef AREA52_HUNTER_TALENT_RULES_H
#define AREA52_HUNTER_TALENT_RULES_H
#include <cstddef>
#include <cstdint>
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
