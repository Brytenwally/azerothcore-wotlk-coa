/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AllSpellScript.h"
#include "Area52HunterTalentRules.h"
#include "AscensionFreePick.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"

namespace
{
using namespace Area52HunterTalents;

class area52_focused_fire_crit : public AllSpellScript
{
public:
    area52_focused_fire_crit() : AllSpellScript("area52_focused_fire_crit", {ALLSPELLHOOK_ON_CRIT_CHANCE}) { }

    void OnSpellCritChance(Spell* spell, Unit* target, float& chance) override
    {
        Unit* pet = spell->GetCaster();
        SpellInfo const* info = spell->GetSpellInfo();
        if (!target || !pet || !pet->IsPet() || !info || info->SpellFamilyName != SPELLFAMILY_HUNTER ||
            !FocusedFireApplies(info->SpellFamilyFlags[1], pet->HasAura(KillCommandPetAura)))
            return;
        Player* owner = pet->GetOwner() ? pet->GetOwner()->ToPlayer() : nullptr;
        if (!AscensionFreePick::Applies(owner))
            return;
        if (AuraEffect const* focusedFire = owner->GetAuraEffectOfRankedSpell(FocusedFire, EFFECT_1))
            chance += focusedFire->GetAmount();
    }
};
}

void AddSC_Area52HunterBeastMasteryTalents()
{
    new area52_focused_fire_crit();
}
