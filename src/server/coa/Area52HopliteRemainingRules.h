#ifndef AREA52_HOPLITE_REMAINING_RULES_H
#define AREA52_HOPLITE_REMAINING_RULES_H
#include <algorithm>
#include <cstdint>
namespace Area52HopliteRemaining
{
enum Spell : std::uint32_t
{
    HungerAbility = 51662,
    HungerBuff = 63848,
    HungerSpeedBuff = 863849,
    CriticalBlockEnchant = 947294,
    CriticalBlockRank1 = 47294,
    CriticalBlockRank2 = 47295,
    CriticalBlockRank3 = 47296,
    TouchedRank1 = 53590,
    TouchedRank2 = 53591,
    TouchedRank3 = 53592,
    WindSlamEnchant = 979187,
    WindSlamBuff = 979188,
    WindSlamDamage = 979189,
    WindWithinBuff = 979190,
    DraconicSinister = 782246,
    DraconicProc = 782247,
    DraconicSpellCrit = 782200,
    DraconicWeaponCrit = 782201,
    WorldforgedCooldown = 780999,
    ShieldSlam = 23922,
    ShieldOfRighteousness = 53600,
    HopliteShieldStrike = 978533,
    Slam = 1464,
    JudgementLight = 20271,
    JudgementJustice = 53407,
    JudgementWisdom = 53408,
    JudgementDamage = 54158,
    JudgementRighteousness = 20187,
    JudgementCommand = 20467,
    JudgementVengeance = 31804,
    JudgementCorruption = 53733,
    LegacyShieldStrike = 300158,
    SinisterStrike = 1752,
    Thrust = 978569,
};
inline std::uint32_t HungerDuration(std::uint32_t bleeds) { return 5000u * std::min(bleeds, 100u); }
inline std::int32_t HungerSpeed(std::uint32_t bleeds) { return 15 * std::min(bleeds, 100u); }
inline std::uint32_t AdditionalBlock(std::uint32_t damage, std::uint32_t block, std::int32_t percent)
{
    return damage > block ? std::uint64_t(damage - block) * std::clamp(percent, 0, 100) / 100 : 0;
}
inline bool ShieldConsumer(std::uint32_t root) { return root == ShieldSlam || root == ShieldOfRighteousness || root == HopliteShieldStrike; }
inline bool WindConsumer(std::uint32_t root)
{
    return ShieldConsumer(root) || root == Slam || root == JudgementLight || root == JudgementJustice || root == JudgementWisdom ||
        root == JudgementDamage || root == JudgementRighteousness || root == JudgementCommand || root == JudgementVengeance || root == JudgementCorruption || root == LegacyShieldStrike;
}
inline bool DraconicConsumer(std::uint32_t root) { return root == SinisterStrike || root == Thrust; }
}
#endif
