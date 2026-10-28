bool HasSpellScriptNamed(uint32 spell, char const* name)
{
    std::list<SpellScript*> scripts;
    sScriptMgr->CreateSpellScripts(spell, scripts);
    bool found = false;
    for (auto* script : scripts)
    {
        found |= *script->_GetScriptName() == name;
        delete script;
    }
    return found;
}

bool HasArea52DragonWarriorHandlers(uint32 spell)
{
    if (spell == Area52DragonWarrior::DragonWarrior)
        return sSpellMgr->GetSpellProcEntry(spell) && sSpellMgr->GetSpellProcEntry(Area52DragonWarrior::DraconicSlam) &&
            sSpellMgr->GetSpellInfo(Area52DragonWarrior::DragonRoar) &&
            HasSpellScriptNamed(Area52DragonWarrior::DragonFury, "spell_area52_dragon_fury") &&
            HasSpellScriptNamed(Area52DragonWarrior::DragonRoarDummy, "spell_area52_dragon_roar_stun");
    return spell == Area52DragonWarrior::UnstoppableForce;
}

class spell_area52_dragon_fury : public SpellScript
{
    PrepareSpellScript(spell_area52_dragon_fury);

    void CutRoarCooldown()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player && AscensionFreePick::Applies(player))
            player->ModifySpellCooldown(Area52DragonWarrior::DragonRoar, -Area52DragonWarrior::FuryCooldownCutMs);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_dragon_fury::CutRoarCooldown);
    }
};

class spell_area52_dragon_roar_stun : public SpellScript
{
    PrepareSpellScript(spell_area52_dragon_roar_stun);

    void Stun(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Aura const* fury = caster ? caster->GetAura(Area52DragonWarrior::DragonFury) : nullptr;
        if (!Area52DragonWarrior::RoarStuns(fury ? fury->GetStackAmount() : 0))
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnEffectLaunch += SpellEffectFn(spell_area52_dragon_roar_stun::Stun, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_dragon_roar_stun::Stun, EFFECT_1, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class area52_unstoppable_force : public AllSpellScript
{
public:
    area52_unstoppable_force() : AllSpellScript("area52_unstoppable_force", {ALLSPELLHOOK_ON_CRIT_CHANCE}) { }

    void OnSpellCritChance(Spell* spell, Unit* target, float& chance) override
    {
        Player* player = spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        SpellInfo const* info = spell->GetSpellInfo();
        if (!target || !player || !AscensionFreePick::Applies(player) || info->SpellFamilyName != SPELLFAMILY_WARRIOR ||
            !(info->SpellFamilyFlags[0] & Area52DragonWarrior::SlamFamilyMask0) ||
            !player->HasAura(Area52DragonWarrior::UnstoppableForce))
            return;
        constexpr uint64 incapacitated = (1ULL << MECHANIC_KNOCKOUT) | (1ULL << MECHANIC_SAPPED) |
            (1ULL << MECHANIC_POLYMORPH) | (1ULL << MECHANIC_SLEEP) | (1ULL << MECHANIC_SHACKLE) |
            (1ULL << MECHANIC_STUN);
        constexpr uint64 immobilized = (1ULL << MECHANIC_ROOT) | (1ULL << MECHANIC_FREEZE);
        if (Area52DragonWarrior::SlamAlwaysCrits(target->HasAuraWithMechanic(incapacitated) ||
                target->HasAuraType(SPELL_AURA_MOD_STUN),
                target->HasAuraWithMechanic(immobilized) || target->HasAuraType(SPELL_AURA_MOD_ROOT)))
            chance = 100.0f;
    }
};
