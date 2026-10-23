#include "AscensionPowerSurge.h"
#include "AscensionPowerSurgeRules.h"
#include "AscensionFreePick.h"

namespace AscensionPowerSurge
{
struct State : DataMap::Base
{
    ReturnState mana;
    bool dispatching = false;
};

unsigned Rank(Unit* unit)
{
    if (unit->HasAura(828576)) return 3;
    if (unit->HasAura(828575)) return 2;
    if (unit->HasAura(828574)) return 1;
    return 0;
}

void Damage(Unit* victim, Unit* attacker, std::uint32_t damage, std::uint32_t absorbed, bool melee)
{
    Player* player = victim ? victim->ToPlayer() : nullptr;
    if (!player || !AscensionFreePick::Applies(player) || !player->IsAlive())
        return;
    unsigned rank = Rank(player);
    if (!rank)
        return;
    Spell* cast = player->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
    if (!cast)
        cast = player->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (!cast || cast->getState() == SPELL_STATE_FINISHED)
        return;
    auto const* info = cast->GetSpellInfo();
    State* state = player->CustomData.GetDefault<State>("area52.power_surge");
    if (rank == 3 && info->Id == 986067)
        state->mana.Capture(damage, absorbed);
    if (!melee || !attacker || attacker == victim || (!damage && !absorbed) || state->dispatching)
        return;
    if (!info->IsChanneled() && !info->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE) &&
        !info->HasEffect(SPELL_EFFECT_HEALTH_LEECH) && !info->HasAura(SPELL_AURA_PERIODIC_DAMAGE))
        return;
    unsigned spell = Burst(info->SchoolMask);
    if (!spell || !roll_chance_i(Chance(rank)))
        return;
    state->dispatching = true;
    player->CastSpell(player, spell, true);
    state->dispatching = false;
}

void Finish(Unit* unit, std::uint32_t spell, bool completed)
{
    if (spell != 986067 || !unit)
        return;
    if (State* state = unit->CustomData.Get<State>("area52.power_surge"))
        state->mana.Finish(completed && Rank(unit) == 3 && unit->IsAlive());
}

void Update(Unit* unit, std::uint32_t diff)
{
    State* state = unit->CustomData.Get<State>("area52.power_surge");
    if (!state)
        return;
    if (!unit->IsAlive() || Rank(unit) != 3)
    {
        state->mana = {};
        return;
    }
    if (!state->mana.ticks)
        return;
    state->mana.elapsed += diff;
    while (state->mana.elapsed >= 1000 && state->mana.ticks)
    {
        state->mana.elapsed -= 1000;
        auto amount = state->mana.Tick();
        auto maximum = unit->GetMaxPower(POWER_MANA);
        auto current = unit->GetPower(POWER_MANA);
        auto missing = maximum > current ? maximum - current : 0;
        unit->EnergizeBySpell(unit, 828573, std::min<std::uint64_t>(amount, missing), POWER_MANA);
    }
}
}
