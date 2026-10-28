#ifndef AREA52_MARKSMAN_RULES_H
#define AREA52_MARKSMAN_RULES_H
#include <cstdint>

namespace Area52Marksman
{
constexpr std::uint32_t CompoundPower = 979740;
constexpr std::uint32_t CompoundFlurry = 984926;
constexpr std::uint32_t CompoundShot = 984925;
constexpr std::uint32_t Sniper = 81166;
constexpr std::uint32_t SniperStack = 81167;
constexpr std::uint32_t SniperStackSeconds = 3;
constexpr std::uint32_t SteadyShot = 56641;
constexpr std::uint32_t ArcaneShot = 3044;
constexpr std::uint32_t MultiShot = 2643;
constexpr std::uint32_t ChimeraShot = 53209;
constexpr std::uint32_t SliceAndDice = 5171;

struct ShotEffect
{
    std::int8_t ComboPoints = 0;
    bool Empowers = false;
    bool Consumes = false;
};

inline ShotEffect CompoundPowerShot(std::uint32_t firstRank)
{
    ShotEffect effect;
    effect.ComboPoints = firstRank == SteadyShot || firstRank == ArcaneShot ? 2 : 0;
    effect.Empowers = firstRank == MultiShot || firstRank == ArcaneShot || firstRank == ChimeraShot ||
        firstRank == CompoundShot;
    effect.Consumes = firstRank == SteadyShot;
    return effect;
}

inline bool Triggers(ShotEffect effect)
{
    return effect.ComboPoints || effect.Empowers || effect.Consumes;
}

struct SniperStep
{
    std::uint32_t Still = 0;
    bool AddStack = false;
    bool Clear = false;
};

inline SniperStep SniperTick(std::uint32_t stillSeconds, bool moving)
{
    if (moving)
        return {0, false, true};
    std::uint32_t const still = stillSeconds + 1;
    return {still, still % SniperStackSeconds == 0, false};
}
}
#endif
