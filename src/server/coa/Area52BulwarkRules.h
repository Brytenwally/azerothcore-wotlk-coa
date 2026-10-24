#ifndef AREA52_BULWARK_RULES_H
#define AREA52_BULWARK_RULES_H
#include <algorithm>
#include <array>
#include <cstdint>

namespace Area52Bulwark
{
inline constexpr std::array<std::uint32_t, 6> TankStances = {71, 48263, 25780, 701463, 5487, 9634};

inline std::uint32_t EnergyDrain(std::uint32_t elapsedTicks)
{
    return 10u + 5u * std::min(elapsedTicks, 10u);
}

inline bool WildAggressionEligible(std::uint32_t root, bool ownDamage, std::uint32_t damage)
{
    return root == 1495 && ownDamage && damage > 0;
}

inline bool InsightEffect(std::uint8_t index, bool playerAttacker)
{
    return index == (playerAttacker ? 2 : 0);
}
}
#endif
