/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionMysticEnchantCatalog.h"
#include "ClientDBC.h"
#include <algorithm>
#include <string_view>
#include <vector>

namespace AscensionMysticEnchant
{
bool LoadCatalog(Catalog& catalog, std::string const& path)
{
    catalog = {};
    ClientDBC data;
    if (!data.Load(path, 31) || data.GetRecordSize() != 124)
        return false;
    constexpr std::array<std::string_view, 7> qualities = { "RE_QUALITY_POOR", "RE_QUALITY_NORMAL",
        "RE_QUALITY_UNCOMMON", "RE_QUALITY_RARE", "RE_QUALITY_EPIC", "RE_QUALITY_LEGENDARY",
        "RE_QUALITY_ARTIFACT" };
    Catalog loaded;
    for (std::uint32_t i = 0; i < data.GetRecordCount(); ++i)
    {
        auto const row = data.GetRecord(i);
        if (!row.GetUInt32(8) || !(row.GetUInt32(13) & (1u << 9)))
            continue;
        auto const quality = std::find(qualities.begin(), qualities.end(), row.GetString(3));
        if (quality == qualities.end() || !row.GetUInt32(1))
            return false;
        Entry entry;
        entry.Id = row.GetUInt32(0);
        entry.Spell = row.GetUInt32(1);
        entry.Quality = std::uint32_t(quality - qualities.begin());
        entry.Level = row.GetUInt32(5);
        entry.Item = row.GetUInt32(6);
        entry.Worldforged = row.GetUInt32(7) != 0;
        for (std::uint32_t j = 0; j < 3; ++j)
        {
            entry.Classes[j] = row.GetUInt32(19 + j);
            entry.Tabs[j] = row.GetUInt32(22 + j);
            entry.AbilityInvestment[j] = row.GetUInt32(25 + j);
            entry.TalentInvestment[j] = row.GetUInt32(28 + j);
        }
        if (!loaded.BySpell.emplace(entry.Spell, entry).second)
            return false;
        if (entry.Item && !loaded.ByItem.emplace(entry.Item, entry.Spell).second)
            return false;
    }
    catalog = std::move(loaded);
    return !catalog.BySpell.empty();
}

bool ResolveNative(std::uint32_t spell, std::map<std::uint32_t, SpellNode> const& spells)
{
    std::vector<std::uint32_t> pending = { spell };
    std::set<std::uint32_t> visited;
    while (!pending.empty())
    {
        auto const id = pending.back();
        pending.pop_back();
        if (!visited.insert(id).second)
            continue;
        auto const found = spells.find(id);
        if (found == spells.end() || !found->second.Supported)
            return false;
        pending.insert(pending.end(), found->second.Dependencies.begin(), found->second.Dependencies.end());
    }
    return true;
}

bool IsRemovalOnly(Slots const& before, Slots const& wanted)
{
    bool removed = false;
    for (std::size_t i = 0; i < wanted.size(); ++i)
    {
        if (wanted[i] && wanted[i] != before[i])
            return false;
        removed |= before[i] && !wanted[i];
    }
    return removed;
}

bool ValidateSlots(Catalog const& catalog, Slots const& slots, std::uint32_t level,
    std::set<std::uint32_t> const& owned, std::map<std::uint32_t, std::uint32_t> const& stackLimits)
{
    std::array<std::uint32_t, 3> qualities{};
    std::map<std::uint32_t, std::uint32_t> stacks;
    for (auto spell : slots)
    {
        if (!spell)
            continue;
        auto const found = catalog.BySpell.find(spell);
        auto const limit = stackLimits.find(spell);
        if (found == catalog.BySpell.end() || !owned.contains(spell) || limit == stackLimits.end() ||
            ++stacks[spell] > limit->second)
            return false;
        Entry const& entry = found->second;
        if (entry.Level > level || entry.Quality < 2 || entry.Quality > 5)
            return false;
        ++qualities[entry.Quality < 4 ? 0 : entry.Quality - 3];
    }
    return qualities[1] + qualities[2] <= 4 && qualities[2] <= 1;
}
}
