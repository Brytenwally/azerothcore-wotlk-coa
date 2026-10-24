#ifndef AREA52_BUILD_SELECTION_H
#define AREA52_BUILD_SELECTION_H

#include "Area52BuildProtocol.h"
#include "AscensionFreePick.h"
#include <algorithm>
#include <map>
#include <set>

namespace Area52Build
{
inline AscensionFreePick::Result Select(AscensionFreePick::Catalog const& catalog,
    Entry const& plan, std::uint32_t level, std::vector<AscensionFreePick::KnownEntry>& output,
    std::vector<AscensionFreePick::KnownEntry> const& previous = {}, bool retainExisting = false,
    std::uint32_t maximumLevel = 80, bool learnSpells = true)
{
    std::map<std::uint32_t, std::uint32_t> ranks;
    std::set<std::uint32_t> seen;
    for (auto const& spell : learnSpells ? std::span(plan.Spells) : std::span<Spell const>{})
    {
        if (!spell.Id || !spell.Level || spell.Level > 80 || !seen.insert(spell.Id).second)
            return { "CA_UPDATE_ENTRIES_BAD_ENTRY", "CA_LEARN_BAD_ABILITY" };
        AscensionFreePick::Entry const* found = nullptr;
        std::uint32_t rank = 0;
        for (auto const& [id, entry] : catalog.Entries)
        {
            auto it = std::find(entry.Spells.begin(), entry.Spells.end(), spell.Id);
            if (it == entry.Spells.end())
                continue;
            if (found)
                return { "CA_UPDATE_ENTRIES_BAD_ENTRY", "CA_LEARN_BAD_ABILITY" };
            found = &entry;
            rank = std::uint32_t(it - entry.Spells.begin() + 1);
        }
        if (!found)
            return { "CA_UPDATE_ENTRIES_BAD_ENTRY", "CA_LEARN_BAD_ABILITY" };
        if (spell.Level <= level && spell.Level <= maximumLevel && found->Level <= maximumLevel)
            ranks[found->Id] = std::max(ranks[found->Id], rank);
    }
    if (retainExisting)
        for (auto const& known : previous)
            ranks[known.EntryId] = std::max(ranks[known.EntryId], known.Rank);
    {
        constexpr std::array<std::uint32_t, 7> paths = { 1149, 1150, 0, 1151, 1152, 0, 18149 };
        if (plan.PrimaryStat >= paths.size() || !paths[plan.PrimaryStat]
            || !catalog.Entries.contains(paths[plan.PrimaryStat]))
            return { "CA_UPDATE_ENTRIES_BAD_ENTRY", "CA_LEARN_BAD_ABILITY" };
        for (auto path : paths)
            if (path)
                ranks.erase(path);
        ranks[paths[plan.PrimaryStat]] = 1;
    }
    std::vector<AscensionFreePick::KnownEntry> wanted;
    for (auto const& [id, rank] : ranks)
        wanted.push_back({ id, rank, rank, false, std::uint32_t(wanted.size()) });
    auto result = AscensionFreePick::Validate(catalog, level, wanted, previous);
    if (result)
        output = std::move(wanted);
    return result;
}
}

#endif
