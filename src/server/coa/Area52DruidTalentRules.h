#ifndef AREA52_DRUID_TALENT_RULES_H
#define AREA52_DRUID_TALENT_RULES_H
#include <cstddef>
#include <cstdint>
#include <optional>

namespace Area52DruidTalents
{
constexpr std::uint32_t NaturesReach[] = {16819, 16820, 816819};
constexpr std::uint32_t Moonglow[] = {16845, 16846, 16847};
constexpr std::uint32_t MoonglowBuff = 965818;
constexpr std::uint32_t Vengeance = 16909;
constexpr std::uint32_t CelestialBane = 57849;
constexpr std::uint32_t CelestialFocusCapstone = 16924;
constexpr std::uint32_t InterruptImmunity = 54748;

constexpr std::uint32_t ImprovedMoonfireCapstone = 816823;
constexpr std::uint32_t BramblesCapstone = 16840;
constexpr std::uint32_t BramblesSilence = 816841;
constexpr std::uint32_t MoonfuryCapstone = 16899;
constexpr std::uint32_t MoonfuryEmpowered = 16900;
constexpr std::uint32_t VengeanceCapstone = 16913;
constexpr std::uint32_t VengefulSoul = 816913;
constexpr std::uint32_t VengefulSoulDeparture = 816914;
constexpr std::uint32_t WrathOfCenariusCapstone = 33605;
constexpr std::uint32_t SolarBeam = 954523;
constexpr std::uint32_t NaturesGraceBuff = 16886;
constexpr std::uint32_t CelestialBaneCapstone = 57851;
constexpr std::uint32_t NatureMastery = 857851;
constexpr std::uint32_t NaturesSplendorCapstone = 857866;
constexpr std::uint32_t SplendorDurations = 857867;
constexpr std::uint32_t SplendorFlourishAndInsectSwarm = 857868;
constexpr std::uint32_t Flourish[] = {760174, 760175};
constexpr std::uint32_t NaturesReachCapstone = 816819;
constexpr std::uint32_t InsectSwarm = 5570;
constexpr std::uint32_t Moonfire = 8921;

constexpr std::uint32_t WrathFlag0 = 0x1;
constexpr std::uint32_t MoonfireFlag0 = 0x2;
constexpr std::uint32_t EntanglingRootsFlag0 = 0x200;
constexpr std::uint32_t InsectSwarmFlag0 = 0x200000;

constexpr std::uint32_t MoonfireExtraTickCooldownMs = 1500;
constexpr std::uint32_t BramblesSilenceCooldownMs = 10000;
constexpr std::int32_t WrathOfCenariusStepMs = 1000;
constexpr float InsectSwarmSpreadRadius = 5.0f;
constexpr std::size_t InsectSwarmSpreadMaxTargets = 10;
constexpr std::uint32_t VengefulSoulMissingManaPct = 10;

constexpr std::int32_t VengeanceBasePct = 60;
constexpr std::int32_t VengeancePctPerRank = 10;

template <std::size_t N>
constexpr bool Contains(std::uint32_t const (&ids)[N], std::uint32_t id)
{
    for (std::uint32_t entry : ids)
        if (entry == id)
            return true;
    return false;
}

inline std::optional<bool> ModifierTarget(std::uint32_t modifier, std::uint32_t target, std::uint32_t firstRank,
    bool castDamageSpell)
{
    if (Contains(NaturesReach, modifier))
        return castDamageSpell;
    if (Contains(Moonglow, modifier))
        return target == MoonglowBuff;
    if (modifier == SplendorFlourishAndInsectSwarm)
        return Contains(Flourish, target) || firstRank == InsectSwarm;
    return std::nullopt;
}

inline std::uint32_t VengefulSoulMana(std::uint32_t maxMana, std::uint32_t mana)
{
    return mana < maxMana ? (maxMana - mana) * VengefulSoulMissingManaPct / 100 : 0;
}

inline bool ShouldSpreadInsectSwarm(std::int32_t sourceRemainingMs, std::int32_t victimRemainingMs, bool victimHasIt)
{
    return !victimHasIt || victimRemainingMs < sourceRemainingMs;
}

inline std::int32_t VengeanceCritBonusPct(std::uint8_t rank)
{
    return rank ? VengeanceBasePct + VengeancePctPerRank * (rank - 1) : 0;
}
}
#endif
