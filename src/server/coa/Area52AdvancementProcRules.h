#ifndef AREA52_ADVANCEMENT_PROC_RULES_H
#define AREA52_ADVANCEMENT_PROC_RULES_H
#include <cstdint>
namespace Area52AdvancementProcs
{
struct Event
{
    bool FreePick = false;
    bool OwnerIsActor = false;
    bool Damage = false;
    bool Periodic = false;
    bool Dodged = false;
    bool Behind = false;
    std::uint32_t SpellRoot = 0;
    std::uint32_t Family = 0;
    std::uint32_t School = 0;
};
inline bool Accept(std::uint32_t aura, Event const& event)
{
    if (!event.FreePick)
        return false;
    if (aura == 12753)
        return !event.OwnerIsActor && event.Dodged;
    if (!event.OwnerIsActor)
        return false;
    switch (aura)
    {
        case 12308: case 12810: case 12811:
            return event.Damage && !event.Periodic && event.Family == 4;
        case 14142:
            return event.Damage && event.Periodic &&
                (event.SpellRoot == 703 || event.SpellRoot == 1943 || event.SpellRoot == 954809);
        case 14769:
            return !event.Periodic && event.SpellRoot == 17;
        case 816968:
            return event.Damage && !event.Periodic && event.Behind &&
                (event.SpellRoot == 5221 || event.SpellRoot == 6785);
        case 31682: case 31683: case 31684:
            return !event.Periodic && event.SpellRoot == 116;
        case 33225:
            return event.Damage && event.Periodic && (event.School & (2 | 32));
        default:
            return false;
    }
}
}
#endif
