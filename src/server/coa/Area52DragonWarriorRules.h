#ifndef AREA52_DRAGON_WARRIOR_RULES_H
#define AREA52_DRAGON_WARRIOR_RULES_H
#include <cstdint>

namespace Area52DragonWarrior
{
constexpr std::uint32_t DragonWarrior = 954252;
constexpr std::uint32_t DraconicSlam = 954261;
constexpr std::uint32_t DragonFury = 954253;
constexpr std::uint32_t DragonRoar = 954250;
constexpr std::uint32_t DragonRoarDummy = 954251;
constexpr std::uint32_t DeathWish = 12292;
constexpr std::uint32_t UnstoppableForce = 911167;
constexpr std::uint32_t SlamFamilyMask0 = 0x200000;
constexpr std::int32_t FuryCooldownCutMs = 5000;
constexpr std::uint8_t RoarStunStacks = 3;

inline bool RoarStuns(std::uint8_t furyStacks)
{
    return furyStacks >= RoarStunStacks;
}

inline bool SlamAlwaysCrits(bool incapacitated, bool immobilized)
{
    return incapacitated || immobilized;
}
}
#endif
