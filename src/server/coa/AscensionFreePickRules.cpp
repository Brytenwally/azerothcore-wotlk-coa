/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionFreePick.h"
#include "ClientDBC.h"
#include "DBCStores.h"
#include <algorithm>
#include <set>
#include <string_view>

namespace AscensionFreePick
{
thread_local SpellLearningScope const* SpellLearningScope::Current = nullptr;

SpellLearningScope::SpellLearningScope(Player const* player, std::set<std::uint32_t> const& spells)
    : Owner(player), Spells(spells), Previous(Current)
{
    Current = this;
}

SpellLearningScope::~SpellLearningScope()
{
    Current = Previous;
}

bool SpellLearningScope::Allows(Player const* player, std::uint32_t spell)
{
    for (auto scope = Current; scope; scope = scope->Previous)
        if (scope->Owner == player)
            return scope->Spells.contains(spell);
    return false;
}

namespace
{
std::uint32_t PackedUInt32(ClientDBC::Record const& row, std::uint32_t offset)
{
    std::uint32_t value = 0;
    for (std::uint32_t byte = 0; byte < 4; ++byte)
        value |= std::uint32_t(row.GetUInt8(offset + byte)) << (byte * 8);
    return value;
}

Result Refuse(char const* reason, std::uint32_t id = 0, std::uint32_t rank = 0)
{
    return { "CA_UPDATE_ENTRIES_NOT_TRAVERSIBLE", reason, id, rank };
}
}

Result::operator bool() const
{
    return std::string_view(Status) == "CA_UPDATE_ENTRIES_OK";
}

bool SameSelection(std::vector<KnownEntry> const& left, std::vector<KnownEntry> const& right)
{
    return left.size() == right.size() && std::is_permutation(left.begin(), left.end(), right.begin(),
        [](KnownEntry const& a, KnownEntry const& b) { return a.EntryId == b.EntryId && a.Rank == b.Rank; });
}

bool LoadCatalog(Catalog& catalog)
{
    catalog = {};
    ClientDBC advancement, classes, essence;
    if (!advancement.Load(GetClientDBCPath("CharacterAdvancement.dbc"), 173) ||
        !classes.Load(GetClientDBCPath("CharacterAdvancementClassTypes.dbc"), 6) ||
        !essence.Load(GetClientDBCPath("CharacterAdvancementEssence.dbc"), 9) ||
        advancement.GetRecordSize() != 692)
        return false;

    std::set<std::uint32_t> allowedClasses;
    for (std::uint32_t index = 0; index < classes.GetRecordCount(); ++index)
    {
        auto const row = classes.GetRecord(index);
        if (row.GetUInt32(2) == 10 || (!row.GetUInt32(2) && row.GetUInt32(3)))
            allowedClasses.insert(row.GetUInt32(0));
    }
    for (std::uint32_t index = 0; index < advancement.GetRecordCount(); ++index)
    {
        auto const row = advancement.GetRecord(index);
        std::uint32_t const flags = PackedUInt32(row, 479);
        std::string_view const type = row.GetString(1);
        std::uint32_t const id = row.GetUInt32(0);
        std::uint32_t const spell = row.GetUInt32(5);
        bool const primaryPath = type == "None" &&
            ((id >= 1149 && id <= 1152 && spell == 84864 + id - 1149) ||
                (id == 18149 && spell == 129243));
        if ((!primaryPath && !allowedClasses.count(row.GetUInt32(32))) || !row.GetUInt8(483) ||
            (flags & (0x1800000 | 0x20000 | 9)) ||
            (!primaryPath && type != "Ability" && type != "Talent" && type != "TalentAbility"))
            continue;

        Entry entry;
        entry.Id = row.GetUInt32(0);
        for (std::uint32_t rank = 0; rank < 9 && row.GetUInt32(5 + rank); ++rank)
            entry.Spells.push_back(row.GetUInt32(5 + rank));
        if (entry.Spells.empty())
            continue;
        for (std::uint32_t i = 0; i < 3; ++i)
        {
            entry.Required[i] = row.GetUInt32(2 + i);
            entry.Masteries[i] = row.GetUInt32(154 + i);
        }
        for (std::uint32_t i = 0; i < 15; ++i)
            if (std::uint32_t const connected = PackedUInt32(row, 415 + 4 * i))
                entry.Connected.push_back(connected);
        for (std::uint32_t i = 0; i < 6; ++i)
            entry.Investment[i] = row.GetUInt32(34 + i);
        entry.Level = row.GetUInt32(26);
        if (type == "Talent")
            entry.Level = std::max(entry.Level, 10 + 5 * row.GetUInt32(31));
        entry.AE = row.GetUInt32(14);
        entry.TE = row.GetUInt32(15);
        constexpr std::array<std::string_view, 4> qualities = { "Uncommon", "Rare", "Epic", "Legendary" };
        auto const quality = std::find(qualities.begin(), qualities.end(), row.GetString(16));
        entry.Quality = quality == qualities.end() ? 0 : std::uint32_t(quality - qualities.begin() + 1);
        entry.QualityCost = row.GetUInt32(17);
        entry.Class = row.GetUInt32(32);
        entry.Tab = row.GetUInt32(33);
        entry.Group = row.GetUInt32(29);
        entry.Flags = flags;
        entry.Points = row.GetUInt32(40);
        entry.RequiredPoints = row.GetUInt32(41);
        catalog.Entries.emplace(entry.Id, std::move(entry));
    }
    for (std::uint32_t index = 0; index < essence.GetRecordCount(); ++index)
    {
        auto const row = essence.GetRecord(index);
        if (row.GetUInt32(2) == 10 && !row.GetUInt32(3) && !row.GetUInt32(4) &&
            !row.GetUInt32(5) && !row.GetUInt32(6))
            catalog.Budgets[row.GetUInt32(1)] = { row.GetUInt32(7), row.GetUInt32(8) };
    }
    return !catalog.Entries.empty() && !catalog.Budgets.empty();
}

Result PlanMasteryRemoval(Catalog const& catalog, std::vector<KnownEntry> const& current,
    std::uint32_t entryId, std::vector<KnownEntry>& wanted)
{
    auto const entry = catalog.Entries.find(entryId);
    if (entry == catalog.Entries.end() ||
        std::none_of(current.begin(), current.end(),
            [&](KnownEntry const& pick) { return pick.EntryId == entryId; }))
        return Refuse("CA_LEARN_BAD_ABILITY", entryId);
    bool const mastery = (entry->second.Flags & 0x1000) != 0;
    if (!mastery && std::none_of(entry->second.Masteries.begin(), entry->second.Masteries.end(),
        [](std::uint32_t id) { return id != 0; }))
        return Refuse("CA_LEARN_BAD_ABILITY", entryId);
    std::set<std::uint32_t> removed = {entryId};
    if (mastery)
    {
        bool changed;
        do
        {
            changed = false;
            for (auto const& pick : current)
            {
                auto const found = catalog.Entries.find(pick.EntryId);
                if (found == catalog.Entries.end())
                    return Refuse("CA_LEARN_BAD_ABILITY", pick.EntryId);
                if (std::any_of(found->second.Masteries.begin(), found->second.Masteries.end(),
                    [&](std::uint32_t id) { return id && removed.contains(id); }))
                    changed = removed.insert(pick.EntryId).second || changed;
            }
        } while (changed);
    }
    wanted.clear();
    for (auto const& pick : current)
        if (!removed.contains(pick.EntryId))
            wanted.push_back(pick);
    return {};
}

std::vector<KnownEntry> IncludeJudgementMastery(std::vector<KnownEntry> wanted)
{
    constexpr std::uint32_t mastery = 1324;
    bool const judgement = std::any_of(wanted.begin(), wanted.end(), [](KnownEntry const& pick)
    {
        return pick.EntryId == 2955 || pick.EntryId == 3019 || pick.EntryId == 3020;
    });
    if (judgement && std::none_of(wanted.begin(), wanted.end(), [](KnownEntry const& pick)
        { return pick.EntryId == mastery; }))
        wanted.push_back({mastery, 1});
    return wanted;
}

Result Validate(Catalog const& catalog, std::uint32_t level, std::vector<KnownEntry> const& requested,
    std::vector<KnownEntry> const& previous)
{
    auto const wanted = IncludeJudgementMastery(requested);
    auto budget = catalog.Budgets.upper_bound(level);
    if (budget == catalog.Budgets.begin() || wanted.size() > catalog.Entries.size())
        return Refuse("CA_LEARN_BAD_ABILITY");
    --budget;
    std::map<std::uint32_t, std::uint32_t> ranks;
    std::set<std::uint32_t> groups;
    std::uint64_t ae = 0, te = 0;
    for (auto const& pick : wanted)
    {
        auto const found = catalog.Entries.find(pick.EntryId);
        if (found == catalog.Entries.end() || !pick.Rank || pick.Rank > found->second.Spells.size() ||
            !ranks.emplace(pick.EntryId, pick.Rank).second)
            return Refuse("CA_LEARN_BAD_ABILITY", pick.EntryId, pick.Rank);
        Entry const& entry = found->second;
        if ((entry.Flags & 0x80000) && std::none_of(previous.begin(), previous.end(),
            [&](KnownEntry const& old) { return old.EntryId == pick.EntryId && old.Rank >= pick.Rank; }))
            return Refuse("CA_LEARN_BAD_ABILITY", pick.EntryId, pick.Rank);
        if (entry.Level > level)
            return Refuse("CA_LEARN_LOW_LEVEL", pick.EntryId, pick.Rank);
        if (entry.Group && !groups.insert(entry.Group).second)
            return Refuse("CA_LEARN_GROUP", pick.EntryId, pick.Rank);
        ae += std::uint64_t(entry.AE) * pick.Rank;
        te += std::uint64_t(entry.TE) * pick.Rank;
    }
    for (auto const& old : previous)
    {
        auto const prior = catalog.Entries.find(old.EntryId);
        auto const target = ranks.find(old.EntryId);
        if (prior == catalog.Entries.end() || (target != ranks.end() && target->second >= old.Rank))
            continue;
        if (prior->second.Flags & 4)
            return Refuse("CA_LEARN_BAD_ABILITY", old.EntryId, old.Rank);
        for (auto const& pick : wanted)
        {
            auto const& masteries = catalog.Entries.at(pick.EntryId).Masteries;
            if (std::find(masteries.begin(), masteries.end(), old.EntryId) != masteries.end())
                return Refuse("CA_LEARN_MISSING_REQUIRED_ID", pick.EntryId, pick.Rank);
        }
    }
    if (ae > budget->second.AE || te > budget->second.TE)
        return Refuse(ae > budget->second.AE ? "CA_LEARN_MISSING_AE" : "CA_LEARN_MISSING_TE");

    std::array<std::uint64_t, 4> qualities{};
    std::map<std::uint32_t, std::uint32_t> learned;
    std::size_t remaining = wanted.size();
    while (remaining)
    {
        bool progressed = false;
        for (auto const& pick : wanted)
        {
            if (learned.count(pick.EntryId))
                continue;
            Entry const& entry = catalog.Entries.at(pick.EntryId);
            bool ready = std::all_of(entry.Required.begin(), entry.Required.end(),
                [&](std::uint32_t id) { return !id || learned.count(id); });
            if (!entry.Connected.empty())
                ready = ready && std::any_of(entry.Connected.begin(), entry.Connected.end(),
                    [&](std::uint32_t id)
                    {
                        auto const dependency = learned.find(id);
                        return dependency != learned.end() && (!(entry.Flags & 0x200000) ||
                            dependency->second == catalog.Entries.at(id).Spells.size());
                    });
            std::array<std::uint64_t, 6> invested{};
            std::uint64_t points = 0;
            for (auto const& [id, rank] : learned)
            {
                Entry const& other = catalog.Entries.at(id);
                for (std::size_t i = 0; i < invested.size(); ++i)
                    if ((i < 2 || other.Class == entry.Class) && (i < 4 || other.Tab == entry.Tab) &&
                        other.Investment[i] < entry.Investment[i])
                        invested[i] += std::uint64_t(i % 2 ? other.TE : other.AE) * rank;
                if (other.Class == entry.Class && other.RequiredPoints < entry.RequiredPoints)
                    points += other.Points;
            }
            for (std::size_t i = 0; i < invested.size(); ++i)
                ready = ready && invested[i] >= entry.Investment[i];
            ready = ready && points >= entry.RequiredPoints;
            if (!ready)
                continue;
            learned.emplace(pick.EntryId, pick.Rank);
            --remaining;
            progressed = true;
        }
        if (!progressed)
            return Refuse("CA_LEARN_MISSING_REQUIRED_ID");
    }
    for (auto const& pick : wanted)
    {
        Entry const& entry = catalog.Entries.at(pick.EntryId);
        bool const mastery = std::any_of(entry.Masteries.begin(), entry.Masteries.end(),
            [&](std::uint32_t id) { return id && ranks.count(id); });
        if (entry.Quality && !mastery)
            qualities[entry.Quality - 1] += entry.QualityCost;
    }
    constexpr std::array<char const*, 4> reasons = { "CA_LEARN_TOO_MANY_UNCOMMON_ABILITIES",
        "CA_LEARN_TOO_MANY_RARE_ABILITIES", "CA_LEARN_TOO_MANY_EPIC_ABILITIES",
        "CA_LEARN_TOO_MANY_LEGENDARY_ABILITIES" };
    for (std::size_t i = 0; i < qualities.size(); ++i)
        if (qualities[i] > QualityLimits[i])
            return Refuse(reasons[i]);
    return {};
}
}
