#ifndef ASCENSION_POWER_SURGE_RULES_H
#define ASCENSION_POWER_SURGE_RULES_H

#include <algorithm>
#include <cstdint>

namespace AscensionPowerSurge
{
inline unsigned Chance(unsigned rank)
{
    return rank <= 3 ? rank * 2 : 0;
}

inline unsigned Burst(unsigned school)
{
    switch (school)
    {
        case 2: return 828597;
        case 4: return 828591;
        case 8: return 828594;
        case 16: return 828595;
        case 32: return 828596;
        case 64: return 828590;
        default: return 0;
    }
}

struct ReturnState
{
    std::uint64_t stored = 0;
    std::uint64_t remaining = 0;
    unsigned ticks = 0;
    unsigned elapsed = 0;

    void Capture(unsigned damage, unsigned absorbed)
    {
        stored += std::uint64_t(damage) + absorbed;
    }

    void Finish(bool completed)
    {
        if (completed && stored)
        {
            remaining += stored;
            ticks = 10;
            elapsed = 0;
        }
        stored = 0;
    }

    std::uint64_t Tick()
    {
        if (!ticks)
            return 0;
        auto amount = remaining / ticks;
        remaining -= amount;
        --ticks;
        return amount;
    }
};
}

#endif
