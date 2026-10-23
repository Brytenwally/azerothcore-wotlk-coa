#ifndef AREA52_ACTIVE_BUILD_H
#define AREA52_ACTIVE_BUILD_H

#include <cstdint>
class Player;
class Creature;
namespace Area52Build
{
struct Entry;
void PauseAutoLearnForReset(Player* player);
bool CanSwitchSpecialization(Player* player);
void RestoreSpecialization(Player* player);
std::uint32_t Train(Player* player, Creature* trainer, std::uint32_t maximumLevel = 80);
}
void SendArea52Advancement(Player* player);
void SendArea52Specialization(Player* player);
#endif
