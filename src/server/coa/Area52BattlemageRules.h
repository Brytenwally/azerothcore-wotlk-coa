#ifndef AREA52_BATTLEMAGE_RULES_H
#define AREA52_BATTLEMAGE_RULES_H
#include <cstdint>

namespace Area52Battlemage
{
inline bool Accept(bool owner, bool damage, bool warrior, bool nextSwing, bool periodic)
{
    return owner && damage && warrior && !nextSwing && !periodic;
}

inline std::int32_t Damage(std::int32_t damage, bool playerTarget)
{
    return playerTarget ? std::int32_t(std::int64_t(damage) * 80 / 100) : damage;
}
}
#endif
