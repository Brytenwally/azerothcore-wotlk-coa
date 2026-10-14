/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_WHITE_WALKER_SUPPORT_RULES_H
#define ASCENSION_WHITE_WALKER_SUPPORT_RULES_H

#include <cstdint>
#include <array>

namespace AscensionWhiteWalkerSupport
{
constexpr std::uint32_t AthleticIcing = 84561;
constexpr std::uint32_t HonedIce = 909019;
constexpr std::uint32_t HonedIceBuff = 909020;
constexpr std::uint32_t WhiteWalker = 978202;
constexpr std::uint32_t AbsoluteZero = 983165;
constexpr std::uint32_t FrostNova = 122;
constexpr std::uint32_t AbsoluteZeroReplacement = 983166;
constexpr std::uint32_t SliceAndDice = 5171;
constexpr std::uint32_t SliceAndIce = 978302;
constexpr std::uint32_t WindsOfWinter = 120;
constexpr std::uint32_t WindsOfWinterReplacement = 978846;
constexpr std::uint32_t IceLance = 30455;
constexpr std::uint32_t IceSpear = 984935;
constexpr std::uint32_t WintersFury = 984936;
constexpr std::uint32_t WintersVitality = 978525;
constexpr std::array<std::uint32_t, 8> WindsOfWinterRanks = { 978846, 978847, 978848, 978849,
    978850, 978851, 978852, 978853 };
constexpr std::array<std::uint32_t, 7> IceSpearRanks = { 984935, 984970, 984971, 984972, 984973, 984974, 984975 };
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

constexpr bool WhiteWalkerActive(bool enchantAura, bool twoHanded, bool knownRoot)
{
    return enchantAura && twoHanded && knownRoot;
}
}

#endif
