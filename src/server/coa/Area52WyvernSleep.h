#ifndef AREA52_WYVERN_SLEEP_H
#define AREA52_WYVERN_SLEEP_H

#include <cstdint>

namespace Area52WyvernSleep
{
constexpr bool PreservesSleep(bool freePick, std::uint32_t sleep, std::uint32_t source,
    std::uint32_t damageRoot)
{
    return freePick && sleep == 24131 && source == 19386 && damageRoot == 1978;
}
}

#endif
