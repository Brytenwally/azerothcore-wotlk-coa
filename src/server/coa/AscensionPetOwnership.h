/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#ifndef ASCENSION_PET_OWNERSHIP_H
#define ASCENSION_PET_OWNERSHIP_H

#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <set>

namespace AscensionPetOwnership
{
inline bool ReachesSummon(uint32 spell, uint32 summon, std::set<uint32>& visited)
{
    if (!spell || !summon || !visited.insert(spell).second)
        return false;

    if (spell == summon || sSpellMgr->GetFirstSpellInChain(spell) == sSpellMgr->GetFirstSpellInChain(summon))
        return true;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(spell);
    if (!info)
        return false;

    if (spell == 31687 && (summon == 70907 || summon == 70908))
        return true;

    if (spell == 46584)
        for (uint8 index : { uint8(EFFECT_1), uint8(EFFECT_2) })
            if (int32 child = info->Effects[index].CalcValue(); child > 0 &&
                ReachesSummon(uint32(child), summon, visited))
                return true;

    for (SpellEffectInfo const& effect : info->Effects)
        if (effect.Effect != SPELL_EFFECT_LEARN_SPELL && effect.TriggerSpell &&
            ReachesSummon(effect.TriggerSpell, summon, visited))
            return true;

    return false;
}

inline bool Authorizes(uint32 spell, uint32 summon)
{
    std::set<uint32> visited;
    return ReachesSummon(spell, summon, visited);
}

inline bool IsOwned(Player const* player, uint32 summon)
{
    for (auto const& [spell, state] : player->GetSpellMap())
        if (player->HasActiveSpell(spell) && Authorizes(spell, summon))
            return true;

    return false;
}
}

#endif
