#ifndef ASCENSION_POWER_SURGE_H
#define ASCENSION_POWER_SURGE_H

#include <cstdint>

class Unit;
namespace AscensionPowerSurge
{
void Damage(Unit* victim, Unit* attacker, std::uint32_t damage, std::uint32_t absorbed, bool melee);
void Update(Unit* unit, std::uint32_t diff);
void Finish(Unit* unit, std::uint32_t spell, bool completed);
}

#endif
