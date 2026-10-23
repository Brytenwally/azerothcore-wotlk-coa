/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_MYSTIC_ENCHANT_H
#define ASCENSION_MYSTIC_ENCHANT_H

#include <cstdint>
#include <string>

class Unit;
class Player;

namespace AscensionMysticEnchant
{
bool PreparePrestigeReset(Player* player, std::uint32_t& specialization, std::string& error);
std::uint32_t ResetPrestigeSpecialization(Player* player);
float NonstackingCostCorrection(Unit const* caster, std::uint32_t schoolMask);
}

#endif
