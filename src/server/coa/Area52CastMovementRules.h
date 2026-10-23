#ifndef AREA52_CAST_MOVEMENT_RULES_H
#define AREA52_CAST_MOVEMENT_RULES_H

#include <cstdint>

namespace Area52CastMovement
{
inline bool RequiresInterruption(std::uint32_t spell)
{
    switch (spell)
    {
        case 331:
        case 332:
        case 403:
        case 421:
        case 529:
        case 547:
        case 548:
        case 596:
        case 913:
        case 915:
        case 930:
        case 939:
        case 943:
        case 959:
        case 996:
        case 1122:
        case 1949:
        case 2860:
        case 2948:
        case 5676:
        case 6041:
        case 6353:
        case 8004:
        case 8005:
        case 8008:
        case 8010:
        case 8444:
        case 8445:
        case 8446:
        case 10205:
        case 10206:
        case 10207:
        case 10391:
        case 10392:
        case 10395:
        case 10396:
        case 10466:
        case 10467:
        case 10468:
        case 10605:
        case 10960:
        case 10961:
        case 11683:
        case 11684:
        case 15207:
        case 15208:
        case 17919:
        case 17920:
        case 17921:
        case 17922:
        case 17923:
        case 17924:
        case 25308:
        case 25316:
        case 25357:
        case 25391:
        case 25396:
        case 25420:
        case 25439:
        case 25442:
        case 25448:
        case 25449:
        case 27073:
        case 27074:
        case 27210:
        case 27211:
        case 27213:
        case 29722:
        case 30459:
        case 30545:
        case 32231:
        case 32546:
        case 42858:
        case 42859:
        case 47814:
        case 47815:
        case 47823:
        case 47824:
        case 47825:
        case 47837:
        case 47838:
        case 48072:
        case 48119:
        case 48120:
        case 49237:
        case 49238:
        case 49270:
        case 49271:
        case 49272:
        case 49273:
        case 49275:
        case 49276:
        case 50796:
        case 51505:
        case 59170:
        case 59171:
        case 59172:
        case 60043:
        case 300946:
        case 300947:
        case 300948:
        case 300949:
        case 954611:
        case 954954:
        case 954955:
        case 954956:
        case 954957:
            return true;
        default:
            return false;
    }
}
}

#endif
