bool HasArea52DragonWarriorHandlers(uint32 spell)
{
    if (spell == Area52DragonWarrior::DragonWarrior)
        return sSpellMgr->GetSpellProcEntry(spell) && sSpellMgr->GetSpellProcEntry(Area52DragonWarrior::DraconicSlam) &&
            sSpellMgr->GetSpellInfo(Area52DragonWarrior::DragonRoar) &&
            sSpellMgr->GetSpellInfo(Area52DragonWarrior::DragonCharge) &&
            HasSpellScriptNamed(Area52DragonWarrior::DragonCharge, "aura_area52_dragon_charge") &&
            HasSpellScriptNamed(Area52DragonWarrior::DragonChargeImpact, "spell_area52_dragon_charge_impact") &&
            HasSpellScriptNamed(Area52DragonWarrior::DragonRoarDummy, "spell_area52_dragon_roar_stun");
    return spell == Area52DragonWarrior::UnstoppableForce;
}

class aura_area52_dragon_charge : public AuraScript
{
    PrepareAuraScript(aura_area52_dragon_charge);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52DragonWarrior::DragonChargeSpeed, Area52DragonWarrior::DragonChargeImpact,
            Area52DragonWarrior::DraconicMight});
    }

    void TickEvery(AuraEffect const*, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = Area52DragonWarrior::ChargeTickMs;
    }

    void Start(AuraEffect const*, AuraEffectHandleModes)
    {
        Unit* charger = GetTarget();
        charger->CastSpell(charger, Area52DragonWarrior::DragonChargeSpeed, true);
        if (Aura* speed = charger->GetAura(Area52DragonWarrior::DragonChargeSpeed))
            speed->SetDuration(GetDuration());
        charger->CastSpell(charger, Area52DragonWarrior::DraconicMight, true);
        if (Aura* might = charger->GetAura(Area52DragonWarrior::DraconicMight))
            might->SetDuration(GetSpellInfo()->Effects[EFFECT_1].CalcValue(charger));
    }

    void Impact(AuraEffect const*)
    {
        Unit* charger = GetTarget();
        float const range = charger->GetCombatReach() + Area52DragonWarrior::ChargeImpactReach;
        std::list<Unit*> candidates;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(charger, charger, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(charger, candidates, check);
        Cell::VisitObjects(charger, searcher, range);
        for (Unit* target : candidates)
        {
            if (!target->IsAlive() || !charger->IsValidAttackTarget(target) || !charger->HasInArc(M_PI / 2, target) ||
                !charger->IsWithinDistInMap(target, Area52DragonWarrior::ChargeImpactReach) ||
                !charger->IsWithinLOSInMap(target))
                continue;
            charger->CastSpell(target, Area52DragonWarrior::DragonChargeImpact, true);
            Remove();
            return;
        }
    }

    void Stop(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(Area52DragonWarrior::DragonChargeSpeed);
    }

    void Register() override
    {
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(aura_area52_dragon_charge::TickEvery, EFFECT_2,
            SPELL_AURA_DUMMY);
        AfterEffectApply += AuraEffectApplyFn(aura_area52_dragon_charge::Start, EFFECT_0,
            SPELL_AURA_FORCE_MOVE_FORWARD, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_dragon_charge::Impact, EFFECT_2, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_dragon_charge::Stop, EFFECT_0,
            SPELL_AURA_FORCE_MOVE_FORWARD, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_dragon_charge_impact : public SpellScript
{
    PrepareSpellScript(spell_area52_dragon_charge_impact);

    void Damage(SpellEffIndex)
    {
        Unit* target = GetHitUnit();
        bool const slowImmune = target && target->IsImmunedToSpellEffect(GetSpellInfo(), EFFECT_2, GetCaster());
        SetHitDamage(Area52DragonWarrior::ImpactDamage(GetHitDamage(), slowImmune));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_dragon_charge_impact::Damage, EFFECT_0,
            SPELL_EFFECT_SCHOOL_DAMAGE);
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
        OnEffectLaunchTarget += SpellEffectFn(spell_area52_dragon_roar_stun::Stun, EFFECT_1,
            SPELL_EFFECT_TRIGGER_SPELL);
    }
};

class area52_unstoppable_force : public AllSpellScript
{
public:
    area52_unstoppable_force() : AllSpellScript("area52_unstoppable_force", {ALLSPELLHOOK_ON_CALCULATED_TARGET}) { }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
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
            hit.crit = true;
    }
};

class spell_area52_intensify_rage : public AuraScript
{
    PrepareAuraScript(spell_area52_intensify_rage);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void CoolDragonRoar(AuraEffect const*, SpellModifier*& mod)
    {
        if (mod)
            mod->mask = GetSpellInfo()->Effects[EFFECT_0].SpellClassMask |
                flag96(0, 0, Area52DragonWarrior::DragonRoarFamilyMask2);
    }

    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_area52_intensify_rage::CoolDragonRoar, EFFECT_0,
            SPELL_AURA_ADD_PCT_MODIFIER);
    }
};
