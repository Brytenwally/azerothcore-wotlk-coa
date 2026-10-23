#ifndef AREA52_STARTER_EQUIPMENT_H
#define AREA52_STARTER_EQUIPMENT_H

#include <array>
#include <cstdint>

namespace Area52StarterEquipment
{
constexpr std::array<std::uint32_t, 12> Items(std::uint8_t race)
{
    std::uint32_t shirt = 38, pants = 39, boots = 40;
    if (race == 2 || race == 5 || race == 6 || race == 8)
    {
        shirt = 6125;
        pants = 139;
        boots = race == 6 || race == 8 ? 0 : 140;
    }
    else if (race == 4)
    {
        shirt = 6120;
        pants = 6121;
        boots = 6122;
    }
    else if (race == 10)
    {
        pants = 20902;
        boots = 20903;
    }
    else if (race == 11)
    {
        shirt = 23473;
        pants = 23474;
        boots = 23475;
    }
    return {shirt, pants, boots, 519243, 25, 2362, 2504, 49778, 35, 2092, 39202, 6948};
}
}

#endif
