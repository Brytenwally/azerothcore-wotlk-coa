/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_MYSTIC_SPELL_GRANTS_H
#define ASCENSION_MYSTIC_SPELL_GRANTS_H
#include <array>
#include <cstdint>

namespace AscensionMysticEnchant
{
struct SpellGrant
{
    std::uint32_t Enchant;
    std::uint32_t Spell;
};
inline constexpr std::array<SpellGrant, 7> SpellGrants = {{
    {81248, 81249},
    {84408, 84409},
    {84466, 84467},
    {92574, 92575},
    {92582, 92584},
    {965937, 965939},
    {978724, 922565}
}};
}
#endif
