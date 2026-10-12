#ifndef ASCENSION_INCARNATION_RULES_H
#define ASCENSION_INCARNATION_RULES_H

#include <cstdint>

namespace AscensionIncarnations
{
inline bool ModelCategory(uint32_t category)
{
    return (category >= 17 && category <= 29) || category == 31;
}

inline uint32_t FormCategory(uint32_t form)
{
    switch (form)
    {
        case 5: case 8: return 17;
        case 1: return 18;
        case 3: return 19;
        case 4: return 20;
        case 27: case 29: return 21;
        case 31: return 22;
        case 2: return 23;
        case 16: return 24;
        case 22: return 31;
        default: return 0;
    }
}

inline uint32_t PetCategory(uint32_t entry)
{
    switch (entry)
    {
        case 416: case 1100416: return 25;
        case 1860: case 1101860: return 26;
        case 1863: case 1101863: return 27;
        case 417: case 1100417: return 28;
        case 17252: case 1117252: return 29;
        default: return 0;
    }
}

inline bool CategoryMatches(uint32_t category, uint32_t primary, uint32_t secondary, uint32_t tertiary)
{
    return category && (category == primary || category == secondary || category == tertiary);
}
}
#endif
