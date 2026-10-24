#ifndef AREA52_LIFE_TAP_H
#define AREA52_LIFE_TAP_H

#include <algorithm>
#include <cstdint>

namespace Area52LifeTap
{
constexpr std::uint32_t Cost(std::uint32_t health, std::uint32_t mana)
{
    return std::max<std::uint32_t>(1, std::uint64_t(std::max(health, mana)) * 6 / 100);
}

constexpr bool CanPay(std::uint32_t currentHealth, std::uint32_t cost)
{
    return currentHealth > cost;
}
}

#endif
