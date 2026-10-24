/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include <algorithm>
#include <array>

namespace
{
constexpr std::array<uint32, 3> VictoryRushSpells = { 34428, 634428, 1134428 };
constexpr uint32 VictoryRushHeal = 965439;
bool Area52Enabled = false;

class AscensionVictoryRushRealm final : public WorldScript
{
public:
    AscensionVictoryRushRealm() : WorldScript("AscensionVictoryRushRealm") { }

    void OnStartup() override
    {
        Area52Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

class aura_wildcard_victorious_state : public AuraScript
{
    PrepareAuraScript(aura_wildcard_victorious_state);

    bool KnowsVictoryRush(ProcEventInfo&)
    {
        Player const* player = GetTarget()->ToPlayer();
        return player && std::any_of(VictoryRushSpells.begin(), VictoryRushSpells.end(),
            [player](uint32 spell) { return player->HasSpell(spell); });
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_wildcard_victorious_state::KnowsVictoryRush);
    }
};

class spell_area52_victory_rush_heal : public SpellScript
{
    PrepareSpellScript(spell_area52_victory_rush_heal);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({VictoryRushHeal}) &&
            sSpellMgr->GetSpellInfo(VictoryRushHeal)->Effects[EFFECT_0].Effect == SPELL_EFFECT_HEAL_PCT;
    }

    void Heal(SpellEffIndex)
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* target = GetHitUnit();
        if (!Area52Enabled || !player || player->getClass() != CLASS_HERO || !target)
            return;
        int32 const percent = target->IsPlayer() ? 8 : 10;
        player->CastCustomSpell(player, VictoryRushHeal, &percent, nullptr, nullptr, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_victory_rush_heal::Heal, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
}

void AddAscensionVictoryRushScripts()
{
    new AscensionVictoryRushRealm();
    RegisterSpellScript(aura_wildcard_victorious_state);
    RegisterSpellScript(spell_area52_victory_rush_heal);
}
