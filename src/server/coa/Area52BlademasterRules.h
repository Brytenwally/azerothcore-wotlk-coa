#ifndef AREA52_BLADEMASTER_RULES_H
#define AREA52_BLADEMASTER_RULES_H

#include <algorithm>
#include <cstdint>

namespace Area52Blademaster
{
constexpr std::uint32_t DeferredDamage(std::uint32_t damage, std::uint32_t bonus, bool playerAttacker)
{
    std::uint32_t percent = std::min(100u, 30u + bonus);
    return std::uint64_t(damage) * percent / (playerAttacker ? 200u : 100u);
}

constexpr std::uint32_t DebtPayment(std::uint32_t debt, std::uint32_t remainingTicks)
{
    return (std::uint64_t(debt) + std::max(1u, remainingTicks) - 1) / std::max(1u, remainingTicks);
}

constexpr bool Eligible(bool weapon, bool offhandWeapon, bool titansGrip, bool touched, bool spirit, bool hoplite)
{
    return weapon && !offhandWeapon && !titansGrip && !touched && !spirit && !hoplite;
}
}

#endif
