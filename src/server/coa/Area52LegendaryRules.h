/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_LEGENDARY_RULES_H
#define AREA52_LEGENDARY_RULES_H
#include <cstdint>

namespace Area52Legendary
{
enum Spells : std::uint32_t
{
    Purification = 84508, Exorcism = 84509, Consecration = 84759, Vulnerability = 984961,
    Curse = 84512, MaledictionDamage = 84760, Stormbringer = 965500, Deathbringer = 965501,
    Zealot = 965502, Smite = 965508
};

inline bool Accept(std::uint32_t enchant, bool area52, bool owner, bool damage, bool periodic, bool area)
{
    if (!area52 || !owner || !damage || periodic)
        return false;
    return enchant == Purification || enchant == Curse || enchant == Stormbringer || enchant == Zealot ||
        (enchant == Deathbringer && !area);
}

inline std::int32_t ZealotAmount(std::int32_t amount, bool playerTarget)
{
    return playerTarget ? amount / 5 : amount;
}
}
#endif
