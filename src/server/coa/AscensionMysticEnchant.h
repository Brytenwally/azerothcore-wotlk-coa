/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_MYSTIC_ENCHANT_H
#define ASCENSION_MYSTIC_ENCHANT_H

#include "AscensionMysticEnchantRules.h"
#include <vector>

class Player;
class Unit;

namespace AscensionMysticEnchant
{
bool ApplyGuide(Player* player, std::vector<GuideEnchant> const& guide);
float NonstackingCostCorrection(Unit const* caster, std::uint32_t schoolMask);
}

#endif
