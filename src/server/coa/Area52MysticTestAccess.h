#ifndef AREA52_MYSTIC_TEST_ACCESS_H
#define AREA52_MYSTIC_TEST_ACCESS_H

#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <string>

namespace AscensionMysticEnchant
{
template <typename Supported>
std::set<std::uint32_t> VerifiedTestAccess(boost::property_tree::ptree const& report, Supported supported)
{
    if (report.get<unsigned>("schema") != 1)
        throw std::runtime_error("Unsupported Mystic Enchant verification schema");
    std::set<std::uint32_t> result;
    std::set<std::uint32_t> seen;
    for (auto const& child : report.get_child("entries"))
    {
        auto const& row = child.second;
        if (row.get<std::string>("kind") != "mystic")
            continue;
        auto const id = row.get<std::uint32_t>("id");
        if (!seen.insert(id).second)
            throw std::runtime_error("Duplicate Mystic Enchant verification entry");
        auto const status = row.get<std::string>("status");
        if ((status == "VERIFIED" || status == "CERTIFIED") && supported(id))
            result.insert(id);
    }
    return result;
}
}

#endif
