#ifndef AREA52_LEGENDARY_BATCH_RULES_H
#define AREA52_LEGENDARY_BATCH_RULES_H

#include <cstdint>

namespace Area52LegendaryBatch
{
inline bool Managed(unsigned spell)
{
    switch (spell)
    {
        case 84510: case 965503: case 965504: case 965505:
        case 81119: case 81122: case 81479: case 965315: case 99634:
            return true;
        default:
            return false;
    }
}

inline bool Direct(bool owner, bool enemy, bool damage, bool periodic, bool triggered)
{
    return owner && enemy && damage && !periodic && !triggered;
}

inline bool Overheating(bool meleeAbility, bool fireMage, unsigned stacks, unsigned maximum)
{
    return meleeAbility || (fireMage && maximum && stacks >= maximum);
}

inline unsigned Clearcast(unsigned root)
{
    return root == 84510 ? 84748 : root == 965503 ? 983908 : 0;
}

inline unsigned Threshold(bool pvp)
{
    return pvp ? 2 : 10;
}
}

#endif
