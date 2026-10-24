#ifndef AREA52_CERTIFICATION_RULES_H
#define AREA52_CERTIFICATION_RULES_H

#include <boost/property_tree/ptree.hpp>
#include <charconv>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <algorithm>
#include <vector>

namespace Area52Certification
{
inline constexpr std::uint32_t GrantedEntryMask = 0x40000000u;
inline constexpr std::uint32_t MysticEntryMask = 0x80000000u;
inline constexpr std::string_view Prefix = "A52CERT\t";

struct TesterScore
{
    std::uint32_t Account = 0;
    std::string Name;
    std::uint32_t Total = 0;
    std::uint32_t Rank = 0;
};

inline void Credit(std::map<std::uint32_t, TesterScore>& scores, std::uint32_t account, std::string name)
{
    auto& score = scores[account];
    score.Account = account;
    score.Name = std::move(name);
    ++score.Total;
}

inline std::vector<TesterScore> Leaderboard(std::map<std::uint32_t, TesterScore> const& scores)
{
    std::vector<TesterScore> result;
    for (auto const& [account, score] : scores)
        result.push_back(score);
    std::sort(result.begin(), result.end(), [](auto const& left, auto const& right)
    {
        return left.Total != right.Total ? left.Total > right.Total : left.Account < right.Account;
    });
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i].Rank = i && result[i].Total == result[i - 1].Total ? result[i - 1].Rank : i + 1;
    return result;
}

inline std::uint32_t ParseLeaderboardPage(std::string_view payload)
{
    if (!payload.starts_with("L|"))
        return 0;
    payload.remove_prefix(2);
    std::uint32_t page = 0;
    auto const [end, error] = std::from_chars(payload.data(), payload.data() + payload.size(), page);
    return error == std::errc{} && end == payload.data() + payload.size() ? page : 0;
}

inline std::map<std::uint32_t, bool> ReadEligibility(boost::property_tree::ptree const& report)
{
    if (report.get<unsigned>("schema") != 1)
        throw std::runtime_error("Unsupported certification ledger schema");
    std::map<std::uint32_t, bool> result;
    std::map<std::uint32_t, bool> seen;
    for (auto const& child : report.get_child("entries"))
    {
        auto const& row = child.second;
        auto const kind = row.get<std::string>("kind");
        if (kind != "advancement" && kind != "mystic" && kind != "granted")
            continue;
        auto id = row.get<std::uint32_t>("id");
        if (!id || id >= GrantedEntryMask)
            throw std::runtime_error("Invalid certification identifier");
        if (kind == "mystic")
            id |= MysticEntryMask;
        else if (kind == "granted")
            id |= GrantedEntryMask;
        if (!seen.emplace(id, true).second)
            throw std::runtime_error("Duplicate certification entry");
        auto const status = row.get<std::string>("status");
        if (status == "VERIFIED" || status == "CERTIFIED")
            result.emplace(id, status == "CERTIFIED" || row.get<bool>("certified", false));
    }
    return result;
}

inline std::map<std::uint32_t, std::vector<std::uint32_t>> ReadGrantedRanks(boost::property_tree::ptree const& report)
{
    std::map<std::uint32_t, std::vector<std::uint32_t>> result;
    for (auto const& child : report.get_child("entries"))
    {
        auto const& row = child.second;
        if (row.get<std::string>("kind") != "granted")
            continue;
        auto const root = row.get<std::uint32_t>("id");
        if (!root || root >= GrantedEntryMask || result.contains(root | GrantedEntryMask))
            throw std::runtime_error("Invalid granted ability identifier");
        auto& ranks = result[root | GrantedEntryMask];
        for (auto const& childRank : row.get_child("ranks"))
        {
            auto const rank = childRank.second.get_value<std::uint32_t>();
            if (!rank || rank >= GrantedEntryMask || ranks.size() >= 100 ||
                std::find(ranks.begin(), ranks.end(), rank) != ranks.end())
                throw std::runtime_error("Invalid granted ability rank");
            ranks.push_back(rank);
        }
        if (std::find(ranks.begin(), ranks.end(), root) == ranks.end())
            throw std::runtime_error("Granted ability root missing from ranks");
    }
    return result;
}

inline std::uint32_t ParseConfirmation(std::string_view payload)
{
    if (!payload.starts_with("C|") || !payload.ends_with("|7"))
        return 0;
    payload.remove_prefix(2);
    payload.remove_suffix(2);
    std::uint32_t id = 0;
    auto const [end, error] = std::from_chars(payload.data(), payload.data() + payload.size(), id);
    return error == std::errc{} && end == payload.data() + payload.size() ? id : 0;
}
}

#endif
