#ifndef AREA52_PORTAL_MASTERY_RULES_H
#define AREA52_PORTAL_MASTERY_RULES_H
#include <algorithm>
#include <array>
#include <cstdint>

namespace Area52PortalMastery
{
inline constexpr std::uint32_t Mastery = 818045;
inline constexpr std::array<std::uint32_t, 26> Spells = {
    3561, 3562, 3563, 3565, 3566, 3567, 10059, 11416, 11417, 11418, 11419, 11420,
    32266, 32267, 32271, 32272, 33690, 33691, 35715, 35717, 49358, 49359, 49360, 49361, 53140, 53142
};
inline bool Contains(std::uint32_t spell)
{
    return std::find(Spells.begin(), Spells.end(), spell) != Spells.end();
}
}
#endif
