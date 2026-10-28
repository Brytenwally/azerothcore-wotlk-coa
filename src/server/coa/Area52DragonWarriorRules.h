#ifndef AREA52_DRAGON_WARRIOR_RULES_H
#define AREA52_DRAGON_WARRIOR_RULES_H
#include <cstdint>

namespace Area52DragonWarrior
{
constexpr std::uint32_t DragonWarrior = 954252;
constexpr std::uint32_t DraconicSlam = 954261;
constexpr std::uint32_t DragonFury = 954253;
constexpr std::uint32_t DragonCharge = 954256;
constexpr std::uint32_t DragonChargeSpeed = 954286;
constexpr std::uint32_t DragonChargeImpact = 954288;
constexpr std::uint32_t DraconicMight = 954289;
constexpr std::uint32_t Intercept = 20252;
constexpr std::uint32_t DragonRoar = 954250;
constexpr std::uint32_t DragonRoarDummy = 954251;
constexpr std::uint32_t DeathWish = 12292;
constexpr std::uint32_t UnstoppableForce = 911167;
constexpr std::uint32_t SlamFamilyMask0 = 0x200000;
constexpr std::int32_t ChargeTickMs = 100;
constexpr float ChargeImpactReach = 2.0f;
constexpr std::int32_t SlowImmuneDamageFactor = 3;
constexpr std::uint8_t RoarStunStacks = 3;

inline bool RoarStuns(std::uint8_t furyStacks)
{
    return furyStacks >= RoarStunStacks;
}

inline std::int32_t ImpactDamage(std::int32_t damage, bool slowImmune)
{
    return slowImmune ? damage * SlowImmuneDamageFactor : damage;
}

inline bool IsThunderSlam(std::uint32_t spell)
{
    return spell == 275048 || (spell >= 275052 && spell <= 275058) ||
        (spell >= 1574554 && spell <= 1574561);
}

inline bool SlamAlwaysCrits(bool incapacitated, bool immobilized, bool thunderSlam = false)
{
    return !thunderSlam && (incapacitated || immobilized);
}
}
#endif
