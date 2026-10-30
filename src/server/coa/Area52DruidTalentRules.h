#ifndef AREA52_DRUID_TALENT_RULES_H
#define AREA52_DRUID_TALENT_RULES_H
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Area52DruidTalents
{
constexpr std::uint32_t NaturesReach[] = {16819, 16820, 816819};
constexpr std::uint32_t Moonglow[] = {16845, 16846, 16847};
constexpr std::uint32_t MoonglowBuff = 965818;
constexpr std::uint32_t Vengeance = 16909;
constexpr std::uint32_t CelestialBane = 57849;
constexpr std::uint32_t CelestialFocusCapstone = 16924;
constexpr std::uint32_t InterruptImmunity = 54748;

constexpr std::int32_t VengeanceBasePct = 60;
constexpr std::int32_t VengeancePctPerRank = 10;

template <std::size_t N>
constexpr bool Contains(std::uint32_t const (&ids)[N], std::uint32_t id)
{
    for (std::uint32_t entry : ids)
        if (entry == id)
            return true;
    return false;
}

inline std::optional<bool> ModifierTarget(std::uint32_t modifier, std::uint32_t target, bool castDamageSpell)
{
    if (Contains(NaturesReach, modifier))
        return castDamageSpell;
    if (Contains(Moonglow, modifier))
        return target == MoonglowBuff;
    return std::nullopt;
}

inline std::int32_t VengeanceCritBonusPct(std::uint8_t rank)
{
    return rank ? VengeanceBasePct + VengeancePctPerRank * (rank - 1) : 0;
}
}
#endif
