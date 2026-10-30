/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef ASCENSION_FREE_PICK_LINKED_SPELLS_H
#define ASCENSION_FREE_PICK_LINKED_SPELLS_H
#include <array>
#include <cstdint>
namespace AscensionFreePick
{
struct LinkedSpellGroup
{
    std::uint32_t Parent;
    std::array<std::uint32_t, 6> Children;
};
inline constexpr std::array<LinkedSpellGroup, 21> LinkedSpellGroups = {{
    {33917, {33876, 33878}},
    {49377, {16979, 49376}},
    {84864, {986202, 986203}},
    {84865, {986200, 986201}},
    {84866, {92839, 92840}},
    {84867, {92842, 92843}},
    {129243, {129245, 129246}},
    {965202, {75}},
    {965200, {1515, 883, 2641, 982, 1462}},
    {891, {885, 889, 893, 109980}},
    {890, {884, 887, 892, 109981}},
    {91634, {91631, 91633, 91652, 109982}},
    {91606, {91602, 91605, 91651, 109983}},
    {5487, {779, 277420}},
    {768, {1082}},
    {48263, {56222}},
    {25780, {277422}},
    {71, {277421}},
    {701463, {277423}},
    {275585, {217364, 275588}},
    {801494, {1494, 19878, 19879, 19880, 19882, 19884}}
}};
}
#endif
