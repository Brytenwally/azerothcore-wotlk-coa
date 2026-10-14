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
constexpr std::uint32_t FreezingBlood = 979135;
constexpr std::uint32_t ColdBlood = 14177;
constexpr std::uint32_t Frostbolt = 116;
constexpr std::uint32_t FightersSinisterStrike = 781046;
constexpr std::uint32_t FightersPower = 781000;
constexpr std::uint32_t SinisterStrike = 1752;
constexpr std::uint32_t TidesOfBattle = 978744;
constexpr std::uint32_t Ebb = 978746;
constexpr std::uint32_t Flow = 978747;

constexpr std::int32_t EviscerateClientCoefficient(std::uint8_t comboPoints)
{
    return 2 * static_cast<std::int32_t>(comboPoints) * static_cast<std::int32_t>(comboPoints);
}
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
constexpr std::array<std::uint32_t, 9> FrostbrandDamageHelpers = { 965990, 965991, 965992, 965993, 965994,
    965995, 965996, 965997, 965998 };
constexpr std::array<std::uint32_t, 9> FrostbrandEnchantments = { 2, 12, 524, 1667, 1668, 2635, 3782, 3783, 3784 };
constexpr std::array<std::uint32_t, 9> FrostbrandSlowHelpers = { 8034, 8037, 10458, 16352, 16353, 965999,
    58797, 58798, 58799 };
constexpr std::int32_t FrozenPowerTalentPercent(std::uint32_t rank)
{
    return rank < 3 ? 3 * static_cast<std::int32_t>(rank + 1) : 0;
}
constexpr std::int32_t FrozenPowerMysticPercent(std::uint32_t copies)
{
    return 2 * static_cast<std::int32_t>(copies > 3 ? 3 : copies);
}
constexpr bool FrozenPowerSchoolAllowed(std::uint32_t schoolMask)
{
    return (schoolMask & (4 | 16 | 8)) != 0;
}
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
