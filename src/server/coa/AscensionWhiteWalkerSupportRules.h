/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_WHITE_WALKER_SUPPORT_RULES_H
#define ASCENSION_WHITE_WALKER_SUPPORT_RULES_H

#include <cstdint>

namespace AscensionWhiteWalkerSupport
{
constexpr std::uint32_t AthleticIcing = 84561;
constexpr std::uint32_t HonedIce = 909019;
constexpr std::uint32_t HonedIceBuff = 909020;
constexpr std::int32_t AthleticCostPercent = -4;
constexpr std::uint32_t AthleticSchools = 1 | 16;

constexpr bool HonedIceSource(std::uint32_t family, std::uint32_t mask, std::uint32_t school,
    bool periodic, std::uint32_t damage, bool owner)
{
    return family == 3 && (mask & 512) && (school & 16) && !periodic && damage && owner;
}

constexpr std::int32_t HonedIceCriticalBonus(std::int32_t amount, std::uint8_t stacks)
{
    return amount * stacks;
}
}

#endif
