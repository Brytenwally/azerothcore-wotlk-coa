/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_MYSTIC_ENCHANT_CATALOG_H
#define ASCENSION_MYSTIC_ENCHANT_CATALOG_H

#include <array>
#include <cstdint>
#include <map>
#include <set>
#include <string>

class Player;

namespace AscensionMysticEnchant
{
struct Entry
{
    std::uint32_t Id = 0;
    std::uint32_t Spell = 0;
    std::uint32_t Item = 0;
    std::uint32_t Level = 0;
    std::uint32_t Quality = 0;
    bool Worldforged = false;
    std::array<std::uint32_t, 3> Classes{};
    std::array<std::uint32_t, 3> Tabs{};
    std::array<std::uint32_t, 3> AbilityInvestment{};
    std::array<std::uint32_t, 3> TalentInvestment{};
};
struct Catalog
{
    std::map<std::uint32_t, Entry> BySpell;
    std::map<std::uint32_t, std::uint32_t> ByItem;
};
bool LoadCatalog(Catalog& catalog, std::string const& path);
struct SpellNode
{
    bool Supported = false;
    std::set<std::uint32_t> Dependencies;
};
bool ResolveNative(std::uint32_t spell, std::map<std::uint32_t, SpellNode> const& spells);
void RefreshEquipped(Player* player);
void RefreshEpicReplacements(Player* player);
bool GrantsActive(Player const* player, std::uint32_t enchant);
void SetGrantedSpell(Player* player, std::uint32_t enchant, std::uint32_t spell, bool enabled);
using Slots = std::array<std::uint32_t, 17>;
bool IsRemovalOnly(Slots const& before, Slots const& wanted);
bool ValidateSlots(Catalog const& catalog, Slots const& slots, std::uint32_t level,
    std::set<std::uint32_t> const& owned, std::map<std::uint32_t, std::uint32_t> const& stackLimits);
}

#endif
