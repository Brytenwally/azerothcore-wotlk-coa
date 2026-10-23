/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_AREA52_ACHIEVEMENT_POLICY_H
#define ASCENSION_AREA52_ACHIEVEMENT_POLICY_H
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace Area52Achievements
{
struct Record
{
    std::uint32_t Id = 0;
    std::uint32_t Category = 0;
    std::string Name;
    std::vector<std::uint32_t> Requirements;
    std::vector<std::vector<std::uint32_t>> Criteria;
    bool RealmFirst = false;
    bool HasConditions = false;
    bool HasRewards = false;
};
inline bool Enabled(bool coa, std::string const& model, std::string const& realm, std::uint32_t mask, bool option)
{
    return coa && option && model == "hero" && realm == "live" && !(mask & ~std::uint32_t(0x400));
}
inline std::string Normalize(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return std::tolower(c); });
    while (!text.empty() && (text.back() == '!' || std::isspace(static_cast<unsigned char>(text.back()))))
        text.pop_back();
    return text;
}
inline bool OtherMode(Record const& record, std::map<std::uint32_t, std::uint32_t> const& categories)
{
    static std::set<std::uint32_t> const excluded = {50, 998, 12000, 15103, 15114, 15124, 15129, 15130,
        15134, 15139, 15140, 16124, 16129, 16130, 16135, 17014, 17015, 17016, 80000};
    std::set<std::uint32_t> visited;
    for (auto category = record.Category; visited.insert(category).second;)
    {
        if (excluded.contains(category))
            return true;
        auto const parent = categories.find(category);
        if (parent == categories.end())
            break;
        category = parent->second;
    }
    auto const name = Normalize(record.Name);
    for (auto const* label : {"nightmare mode", "wildcard", "felforged", "seasonal:", "ironman level",
        "resolute ironman", "survivalist level", "resolute level", "resolute max level"})
        if (name.find(label) != std::string::npos)
            return true;
    return name.starts_with("draft ") || name.starts_with("realm first! draft ");
}
inline std::set<std::uint32_t> Build(std::vector<Record> records,
    std::map<std::uint32_t, std::uint32_t> const& categories)
{
    std::set<std::uint32_t> rejected;
    using Key = std::tuple<std::string, std::vector<std::uint32_t>, std::vector<std::vector<std::uint32_t>>>;
    std::set<Key> families;
    std::sort(records.begin(), records.end(), [](auto const& a, auto const& b) { return a.Id < b.Id; });
    for (auto& record : records)
    {
        if (OtherMode(record, categories))
        {
            rejected.insert(record.Id);
            continue;
        }
        if (!record.RealmFirst || record.Criteria.empty() || record.HasConditions || record.HasRewards)
            continue;
        std::sort(record.Criteria.begin(), record.Criteria.end());
        if (!families.emplace(Normalize(record.Name), record.Requirements, record.Criteria).second)
            rejected.insert(record.Id);
    }
    return rejected;
}
}
#endif
