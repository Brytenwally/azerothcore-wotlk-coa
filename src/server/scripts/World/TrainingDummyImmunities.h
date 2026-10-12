#ifndef TRAINING_DUMMY_IMMUNITIES_H
#define TRAINING_DUMMY_IMMUNITIES_H

#include "Creature.h"
#include "SpellAuraDefines.h"

inline void ApplyTrainingDummyMovementImmunity(Creature* dummy)
{
    for (uint32 effect : {SPELL_EFFECT_KNOCK_BACK, SPELL_EFFECT_KNOCK_BACK_DEST,
        SPELL_EFFECT_PULL, SPELL_EFFECT_PULL_TOWARDS, SPELL_EFFECT_PULL_TOWARDS_DEST})
        dummy->ApplySpellImmune(0, IMMUNITY_EFFECT, effect, true);
    for (uint32 mechanic : {MECHANIC_FEAR, MECHANIC_HORROR, MECHANIC_DISORIENTED, MECHANIC_CHARM})
        dummy->ApplySpellImmune(0, IMMUNITY_MECHANIC, mechanic, true);
    for (uint32 aura : {SPELL_AURA_MOD_FEAR, SPELL_AURA_MOD_CONFUSE, SPELL_AURA_MOD_CHARM,
        SPELL_AURA_MOD_POSSESS, SPELL_AURA_MOD_POSSESS_PET})
        dummy->ApplySpellImmune(0, IMMUNITY_STATE, aura, true);
}

#endif
