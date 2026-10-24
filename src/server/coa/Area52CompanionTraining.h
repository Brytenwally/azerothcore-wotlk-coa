#ifndef AREA52_COMPANION_TRAINING_H
#define AREA52_COMPANION_TRAINING_H
#include <algorithm>
#include <array>
#include <cstdint>

namespace Area52CompanionTraining
{
inline constexpr std::uint32_t Mastery = 818045;
inline constexpr std::array<std::uint32_t, 26> Spells = {
    3561, 3562, 3563, 3565, 3566, 3567, 10059, 11416, 11417, 11418, 11419, 11420,
    32266, 32267, 32271, 32272, 33690, 33691, 35715, 35717, 49358, 49359, 49360, 49361, 53140, 53142
};
inline constexpr std::uint32_t Runeforging = 53428;
inline constexpr std::array<std::uint32_t, 10> Runes = {
    53323, 53331, 53341, 53342, 53343, 53344, 54446, 54447, 62158, 70164
};
inline std::uint32_t Parent(std::uint32_t spell)
{
    if (std::find(Spells.begin(), Spells.end(), spell) != Spells.end())
        return Mastery;
    if (std::find(Runes.begin(), Runes.end(), spell) != Runes.end())
        return Runeforging;
    return 0;
}
inline bool Contains(std::uint32_t spell)
{
    return std::find(Spells.begin(), Spells.end(), spell) != Spells.end();
}
}
#endif
