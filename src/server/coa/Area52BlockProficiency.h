#ifndef AREA52_BLOCK_PROFICIENCY_H
#define AREA52_BLOCK_PROFICIENCY_H

#include "AscensionFreePick.h"
#include "Player.h"
#include "SpellMgr.h"

namespace Area52BlockProficiency
{
inline bool Synchronize(Player* player)
{
    if (!AscensionFreePick::Applies(player))
        return false;
    std::set<std::uint32_t> const spells = { 107, 674 };
    AscensionFreePick::SpellLearningScope learning(player, spells);
    bool changed = false;
    for (uint32 spell : spells)
        if (!player->HasSpell(spell) && sSpellMgr->GetSpellInfo(spell))
        {
            player->learnSpell(spell, false);
            changed = player->HasSpell(spell) || changed;
        }
    return changed;
}
}

#endif
