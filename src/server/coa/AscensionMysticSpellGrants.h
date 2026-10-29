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
inline constexpr std::array<SpellGrant, 24> SpellGrants = {{
    {81248, 81249},
    {84408, 84409},
    {84466, 84467},
    {92574, 92575},
    {92582, 92584},
    {965937, 965939},
    {978724, 922565},
    {978716, 978717},
    {978716, 978718},
    {979740, 826679, 19434},
    {979740, 850515, 19434},
    {982890, 982891, 8676},
    {954252, 954250, 12292, 12292},
    {954252, 954256, 20252, 20252},
    {977906, 947470, 2098},
    {977906, 977908},
    {982965, 982966, 12323},
    {978226, 978617, 1752, 1752},
    {978226, 978618, 2098, 2098},
    {978226, 979023, 48505, 48505},
    {814183, 814184, 14183, 14183},
    {977831, 977832, 8921},
    {86438, 86435, 33891, 33891},
    {86438, 86441, 1064, 1064}
}};
inline constexpr std::array<SpellGrant, 1> RetiredSpellGrants = {{
    {979740, 984925}
}};

struct EnchantUnlock
{
    std::uint32_t Enchant;
    std::uint32_t Spell;
};
inline constexpr std::array<EnchantUnlock, 1> EnchantUnlocks = {{
    {978226, 48525}
}};

inline bool GrantRankDue(bool firstRank, bool known, std::uint32_t spellLevel, std::uint32_t level)
{
    return firstRank || (spellLevel ? spellLevel <= level : known);
}
}
#endif
