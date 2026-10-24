/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#ifndef AREA52_DEPENDENCY_RULES_H
#define AREA52_DEPENDENCY_RULES_H
#include <cstdint>
namespace Area52Dependency
{
constexpr bool NightfallSource(std::uint32_t root)
{
    return root == 172 || root == 689 || root == 1120 || root == 47897;
}
constexpr bool HotStreakSource(std::uint32_t root)
{
    return root == 133 || root == 2136 || root == 2948 || root == 44457 || root == 44461 || root == 44614;
}
constexpr bool FingersSource(std::uint32_t root, std::uint32_t family, std::uint32_t school, bool periodic)
{
    return !periodic && (family == 3 || family == 11) && (school & 16) &&
        root != 10 && root != 42208 && root != 30455;
}
constexpr bool CompleteCriticalPair(std::uint8_t& streak, bool critical)
{
    if (!critical)
    {
        streak = 0;
        return false;
    }
    if (++streak < 2)
        return false;
    streak = 0;
    return true;
}
constexpr std::int32_t DamagePercent(std::uint32_t root, bool player, bool nightfall, bool backlash, bool hotStreak)
{
    std::int32_t percent = 100;
    if (nightfall && root == 686)
        percent = percent * (player ? 75 : 120) / 100;
    if (backlash && (root == 686 || root == 29722))
        percent = percent * (player ? 80 : 110) / 100;
    if (hotStreak && root == 11366 && !player)
        percent = percent * 140 / 100;
    return percent;
}
constexpr std::uint32_t ManaBarrierAbsorb(std::uint32_t damage, std::uint32_t mana, std::uint32_t maximum, bool magic)
{
    if (!maximum)
        return 0;
    return std::uint32_t(std::uint64_t(damage) * 90 * mana / maximum / (magic ? 200 : 100));
}
constexpr std::uint32_t PlayerDamageTakenPercent(bool playerSource, bool barrier, bool bear)
{
    return !playerSource ? 100 : barrier ? 200 : bear ? 115 : 100;
}
constexpr std::uint32_t MissingManaSurge(std::uint32_t mana, std::uint32_t maximum)
{
    return mana >= maximum ? 0 : std::uint32_t(std::uint64_t(maximum - mana) * 30 / 100);
}
}
#endif
