#ifndef AREA52_KNIGHT_OF_THE_ECLIPSE_RULES_H
#define AREA52_KNIGHT_OF_THE_ECLIPSE_RULES_H
#include <algorithm>
#include <cstdint>

namespace Area52KnightOfTheEclipse
{
constexpr std::uint32_t KnightOfTheEclipse = 978226;
constexpr std::uint32_t EclipseStrike = 978617;
constexpr std::uint32_t SolarStrike = 978618;
constexpr std::uint32_t Knightfall = 979023;
constexpr std::uint32_t KnightfallStarfall = 848505;
constexpr std::uint32_t KnightfallStar = 850286;
constexpr std::uint32_t EclipseSolar = 48517;
constexpr std::uint32_t EclipseLunar = 48518;
constexpr std::uint32_t SolarLock = 979037;
constexpr std::uint32_t LunarLock = 979036;
constexpr std::uint32_t KnightHaste = 979022;
constexpr std::uint32_t SolarKnight = 978621;
constexpr std::uint32_t LunarKnight = 978622;
constexpr std::uint32_t SolarBurn = 978623;
constexpr std::uint32_t Sunfire = 977832;
constexpr std::uint32_t SunfireMark = 977833;
constexpr std::uint32_t InfusedEnergy = 979046;
constexpr std::uint32_t InfusedEnergyBuff = 979045;
constexpr std::uint32_t ImprovedInsectSwarm = 957849;
constexpr std::uint32_t DemonicMoonfire = 782152;
constexpr std::uint32_t DemonicMoonfireHelper = 782153;
constexpr std::uint32_t DemonicPower = 781900;
constexpr std::uint32_t WorldforgedCooldown = 780999;
constexpr std::uint32_t InsectSwarmFlag0 = 0x200000;
constexpr std::uint32_t MoonfireFlag0 = 0x2;
constexpr std::int32_t TwoHandedWeaponPct = 80;
constexpr float EclipseStrikeSpellPowerShare = 0.1f;
constexpr std::int32_t SolarBurnChancePerPoint = 20;
constexpr std::int32_t KnightfallMsPerPoint = 600;

enum class Phase : std::uint8_t
{
    None,
    Solar,
    Lunar
};

inline Phase NextPhase(Phase last)
{
    return last == Phase::Solar ? Phase::Lunar : Phase::Solar;
}

inline std::int32_t EclipseStrikeDamage(std::int32_t weaponDamage, bool twoHanded, float spellPower)
{
    std::int32_t const weapon = twoHanded ? std::int32_t(std::int64_t(weaponDamage) * TwoHandedWeaponPct / 100) :
        weaponDamage;
    return weapon + std::int32_t(spellPower * EclipseStrikeSpellPowerShare);
}

inline std::int32_t SolarBurnChance(std::uint8_t comboPoints)
{
    return std::min<std::int32_t>(100, SolarBurnChancePerPoint * comboPoints);
}

inline std::int32_t KnightfallDurationMs(std::uint8_t comboPoints)
{
    return KnightfallMsPerPoint * comboPoints;
}

inline std::int32_t WithBonusPct(std::int32_t damage, std::int32_t bonusPct)
{
    return bonusPct > 0 ? std::int32_t(std::int64_t(damage) * (100 + bonusPct) / 100) : damage;
}
}
#endif
