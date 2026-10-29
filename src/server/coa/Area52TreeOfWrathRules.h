#ifndef AREA52_TREE_OF_WRATH_RULES_H
#define AREA52_TREE_OF_WRATH_RULES_H
#include <array>
#include <cstdint>

namespace Area52TreeOfWrath
{
constexpr std::uint32_t TreeOfWrath = 86438;
constexpr std::uint32_t TreeOfWrathForm = 86435;
constexpr std::uint32_t TreeOfWrathEmpowerment = 86436;
constexpr std::uint32_t LightningRod = 86439;
constexpr std::uint32_t LivingLightning = 86441;
constexpr std::uint32_t InstantLightningBolt = 986438;
constexpr std::uint32_t TreeOfWrathHitChance = 463645;
constexpr std::uint32_t TreeOfLife = 33891;
constexpr std::uint32_t ChainHeal = 1064;
constexpr std::uint32_t LightningBolt = 403;
constexpr std::uint32_t NaturesSwiftness = 16188;
constexpr std::uint32_t NaturesFlow = 983470;
constexpr std::uint32_t NaturesFlowSwiftness = 983469;
constexpr std::uint32_t NaturesFlowCritical = 983472;
constexpr std::uint32_t ChargedTides = 983500;
constexpr std::uint32_t ChargedTidesStack = 983501;
constexpr std::uint32_t BlessingOfTheEternals = 951554;
constexpr std::uint32_t TranscendentalEmbrace = 911127;
constexpr std::uint32_t TranscendentalEmbraceBuff = 411127;
constexpr std::uint32_t LightningOverloadBolt = 45284;
constexpr std::uint32_t LightningOverloadChain = 45297;
constexpr std::array<std::uint32_t, 8> LivingLightningDamage = {{
    86449, 86450, 86451, 86453, 86455, 86456, 86457, 86458
}};

constexpr std::uint32_t ShamanLightningBoltFlag0 = 0x1;
constexpr std::uint32_t ShamanChainLightningFlag0 = 0x2;
constexpr std::uint32_t ShamanHealingWaveFlag0 = 0x40;
constexpr std::uint32_t ShamanLesserHealingWaveFlag0 = 0x80;
constexpr std::uint32_t ShamanChainHealFlag0 = 0x100;
constexpr std::uint32_t DruidHealingTouchFlag0 = 0x20;
constexpr std::uint32_t DruidRegrowthFlag0 = 0x40;
constexpr std::uint32_t LivingLightningCooldownCutFlags0 =
    ShamanHealingWaveFlag0 | ShamanLesserHealingWaveFlag0 | ShamanChainHealFlag0;
constexpr std::uint32_t ChargedTidesSpenderFlags0 = ShamanHealingWaveFlag0 | ShamanChainHealFlag0;
constexpr std::uint32_t TreeOfWrathLightningFlags0 = ShamanLightningBoltFlag0 | ShamanChainLightningFlag0;
constexpr std::uint32_t TranscendentalShamanFlags0 = ShamanHealingWaveFlag0 | ShamanLesserHealingWaveFlag0;
constexpr std::uint32_t TranscendentalDruidFlags0 = DruidHealingTouchFlag0 | DruidRegrowthFlag0;

constexpr std::int32_t LivingLightningCooldownCutMs = 2000;
constexpr std::uint32_t LightningBoltCooldownMs = 6000;
constexpr std::int32_t TreeOfWrathHitChancePct = 100;
constexpr std::int32_t TranscendentalExtraJumps = 1;

inline std::uint32_t CooldownAfterCut(std::uint32_t remainingMs, std::int32_t cutMs)
{
    return remainingMs > std::uint32_t(cutMs) ? remainingMs - std::uint32_t(cutMs) : 0;
}

inline bool IsLivingLightningDamage(std::uint32_t spell)
{
    for (std::uint32_t id : LivingLightningDamage)
        if (id == spell)
            return true;
    return false;
}

inline bool IsLightningOverload(std::uint32_t spell)
{
    return spell == LightningOverloadBolt || spell == LightningOverloadChain;
}

inline std::uint8_t ChargedTidesStacksFor(bool triggered, bool overload)
{
    return overload ? 1 : triggered ? 0 : 1;
}
}
#endif
