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
    std::uint32_t Requires = 0;
    std::uint32_t Replaces = 0;
};
inline constexpr std::array<SpellGrant, 12> SpellGrants = {{
    {81248, 81249},
    {84408, 84409},
    {84466, 84467},
    {92574, 92575},
    {92582, 92584},
    {965937, 965939},
    {978724, 922565},
    {978716, 978717},
    {978716, 978718},
    {979740, 984925, 19434},
    {982890, 982891, 8676},
    {954252, 954250, 12292, 12292}
}};
}
#endif
