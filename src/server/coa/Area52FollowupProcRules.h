#ifndef AREA52_FOLLOWUP_PROC_RULES_H
#define AREA52_FOLLOWUP_PROC_RULES_H
#include <cstdint>
namespace Area52FollowupProcs
{
inline bool HealingOnly(bool freePick, std::uint32_t aura)
{
    return freePick && aura == 760192;
}
inline bool FrostKill(bool freePick, bool capstone, bool frost, bool rewardAllowed, bool eligible)
{
    return freePick && capstone && frost && rewardAllowed && eligible;
}
inline bool WindRush(bool freePick, bool capstone, std::uint32_t root, bool successfulInterrupt)
{
    return freePick && capstone && root == 57994 && successfulInterrupt;
}
inline bool FlameReady(bool capstone, bool cooldown, bool ready)
{
    return capstone && !cooldown && !ready;
}
inline std::int32_t TotemPower(std::uint32_t level, std::uint32_t rank, bool attackPower)
{
    return rank >= 1 && rank <= 3 ? level * rank * (attackPower ? 30 : 15) / 100 : 0;
}
struct Event
{
    bool FreePick = false;
    bool OwnerIsActor = false;
    bool Periodic = false;
    bool Damage = false;
    bool Healing = false;
    bool Critical = false;
    bool OtherTarget = false;
    bool FinalTidalWaves = false;
    std::uint32_t NetherwindStacks = 0;
    std::uint32_t Root = 0;
};
inline std::uint32_t Trigger(std::uint32_t aura, Event const& event)
{
    if (!event.FreePick || !event.OwnerIsActor)
        return 0;
    if (aura == 34493)
        return event.Periodic && event.Damage &&
            (event.Root == 13797 || event.Root == 13812 || event.Root == 3674) ? 734493 : 0;
    if (event.Periodic)
        return 0;
    switch (aura)
    {
        case 16288:
            return event.Root == 2645 ? 965489 : 0;
        case 63506:
            return event.Root == 2061 ? 415011 : 0;
        case 20361:
            return event.Root == 954500 ? 20362 : 0;
        case 19298:
            return event.Damage && event.Critical && (event.Root == 1495 || event.Root == 2973) ? 983791 : 0;
        case 811100:
            return event.Root == 133 ? 811101 : 0;
        case 20042: case 20045:
            return event.Root == 19740 && event.OtherTarget ? 920043 : 0;
        case 851561:
            return event.Root == 379 && event.Healing && event.FinalTidalWaves ? 851562 : 0;
        case 760191:
            return event.Root == 760106 && event.Healing ? 760192 : 0;
        case 44400: case 44402: case 44403:
            if (aura == 44403 && event.Root == 31589 && event.NetherwindStacks >= 3)
                return 844405;
            switch (event.Root)
            {
                case 5143: case 44425: case 30451: case 955032: case 8921: case 2912: case 954800:
                    return aura == 44400 ? 966120 : aura == 44402 ? 966121 : 966122;
                default: return 0;
            }
        case 17958:
            return event.Root == 5676 || event.Root == 29722 || event.Root == 2948 || event.Root == 133 ? 411119 : 0;
        case 12318: case 12857:
            if (event.Root == 6673)
                return aura == 12318 ? 812316 : 812318;
            if (event.Root == 469)
                return aura == 12318 ? 812317 : 812319;
            return 0;
        case 16219:
            return event.Healing && event.Critical &&
                (event.Root == 331 || event.Root == 61295 || event.Root == 8004 || event.Root == 1064) ? 986107 : 0;
        case 863372:
            if (!event.Damage)
                return 0;
            switch (event.Root)
            {
                case 8042: return 863375;
                case 8050: return 863376;
                case 8056: return 863377;
                default: return 0;
            }
        default: return 0;
    }
}
inline std::int32_t TidalExtension(std::uint32_t root)
{
    switch (root)
    {
        case 331: case 61295: return 2000;
        case 8004: return 1000;
        case 1064: return 500;
        default: return 0;
    }
}
}
#endif
