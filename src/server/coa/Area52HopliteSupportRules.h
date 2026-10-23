#ifndef AREA52_HOPLITE_SUPPORT_RULES_H
#define AREA52_HOPLITE_SUPPORT_RULES_H
#include <cstdint>
#include <algorithm>
namespace Area52HopliteSupport
{
inline std::int32_t ExecuteExtraRage(bool freePick, std::int32_t cost, std::int32_t available)
{
    return std::min(std::max(0, (freePick ? 600 : 300) - cost), available);
}
inline float ExecuteAttackPower(bool freePick, float attackPower)
{
    return attackPower * (freePick ? 0.27f : 0.2f);
}
inline float RelentlessChance(float perPoint, std::uint32_t points)
{
    return std::clamp(perPoint * std::min(points, 5u), 0.0f, 100.0f);
}
inline float DeepWoundsTickPercent(bool freePick, std::uint32_t rank)
{
    return (freePick ? 11.0f : 16.0f) * rank / 6.0f;
}
inline bool Enrage(std::uint32_t aura, std::uint32_t root, bool ownerActor, bool damage,
    bool belowHalf, bool periodic)
{
    if (periodic)
        return false;
    if (ownerActor)
        return root == 2687 || root == 18499 || root == 1719;
    return aura == 13048 && damage && belowHalf;
}
inline bool Honor(std::uint32_t aura, bool ownerActor, bool damage, bool periodic, bool hasSpell)
{
    return aura == 51701 && ownerActor && damage && !periodic && hasSpell;
}
inline bool Spartan(std::uint32_t root, bool ownerActor, bool playerTarget, bool stance)
{
    return root == 1766 && ownerActor && playerTarget && stance;
}
}
#endif
