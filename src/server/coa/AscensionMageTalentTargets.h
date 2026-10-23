#ifndef ASCENSION_MAGE_TALENT_TARGETS_H
#define ASCENSION_MAGE_TALENT_TARGETS_H

#include <cstdint>
#include <optional>

inline std::optional<bool> AscensionMageTalentTarget(std::uint32_t modifier, std::uint32_t target)
{
    switch (modifier)
    {
        case 828574:
        case 828575:
        case 828576:
            return false;
        case 31682:
        case 31683:
        case 31684:
            return target == 831682;
        case 54659:
            return target == 701464;
        case 701464:
            return target == 28574 || target == 54658 || target == 54659;
        default:
            return std::nullopt;
    }
}

#endif
