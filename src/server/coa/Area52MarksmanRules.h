#ifndef AREA52_MARKSMAN_RULES_H
#define AREA52_MARKSMAN_RULES_H
#include <cstdint>

namespace Area52Marksman
{
constexpr std::uint32_t CompoundPower = 979740;
constexpr std::uint32_t CompoundFlurry = 984926;
constexpr std::uint32_t CompoundShot = 826679;
constexpr std::uint32_t CompoundShotRecast = 826680;
constexpr std::uint32_t ComboShot = 850515;
constexpr std::uint32_t ComboShotVolley = 850517;
constexpr std::int32_t RecastWindowMs = 3000;
constexpr std::uint32_t SyndicateSniperShotEnchant = 982890;
constexpr std::uint32_t SyndicateSniperShot = 982891;
constexpr std::uint32_t SniperTrainingTalent = 53302;
constexpr std::uint32_t SniperTrainingBuff = 64418;
constexpr std::uint32_t SyndicateSniperTraining = 983867;
constexpr std::uint32_t SyndicateCapstone = 983869;
constexpr std::uint32_t HoneIn = 983887;
constexpr std::uint32_t HoneInDamage = 983888;
constexpr std::uint8_t CapstoneStacks = 4;
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

inline std::uint32_t SyndicateTrainingFor(std::uint32_t talentRank)
{
    return SyndicateSniperTraining + (talentRank - SniperTrainingTalent);
}

inline std::uint32_t SniperTrainingBuffFor(std::uint32_t talentRank)
{
    return SniperTrainingBuff + (talentRank - SniperTrainingTalent);
}

inline std::int32_t WithSniperTraining(std::int32_t damage, std::int32_t bonusPct)
{
    return bonusPct > 0 ? std::int32_t(std::int64_t(damage) * (100 + bonusPct) / 100) : damage;
}

inline bool HonesIn(std::uint8_t capstoneStacks, bool targetPlayerControlled)
{
    return capstoneStacks >= CapstoneStacks && !targetPlayerControlled;
}

inline float WithoutMunitionsHaste(float multiplier, std::int32_t amount)
{
    return amount > 0 ? multiplier * (1.0f + amount / 100.0f) :
        multiplier / (1.0f - amount / 100.0f);
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
