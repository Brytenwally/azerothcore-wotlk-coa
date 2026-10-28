bool HasAuraScriptNamed(uint32 spell, char const* name)
{
    std::list<AuraScript*> scripts;
    sScriptMgr->CreateAuraScripts(spell, scripts);
    bool found = false;
    for (auto* script : scripts)
    {
        found |= *script->_GetScriptName() == name;
        delete script;
    }
    return found;
}

bool HasArea52MarksmanHandlers(uint32 spell)
{
    if (spell == Area52Marksman::CompoundPower)
        return sSpellMgr->GetSpellProcEntry(spell) && sSpellMgr->GetSpellInfo(Area52Marksman::CompoundFlurry) &&
            HasAuraScriptNamed(spell, "spell_area52_compound_power") &&
            HasAuraScriptNamed(Area52Marksman::SliceAndDice, "spell_area52_compound_power_slice_and_dice");
    if (spell == Area52Marksman::Sniper)
        return sSpellMgr->GetSpellInfo(Area52Marksman::SniperStack) && HasAuraScriptNamed(spell, "spell_area52_sniper");
    return false;
}

class spell_area52_compound_power : public AuraScript
{
    PrepareAuraScript(spell_area52_compound_power);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Marksman::CompoundFlurry});
    }

    static Area52Marksman::ShotEffect Effect(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        return spell ? Area52Marksman::CompoundPowerShot(sSpellMgr->GetFirstSpellInChain(spell->Id)) :
            Area52Marksman::ShotEffect{};
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Unit* owner = GetTarget();
        return owner && event.GetActor() == owner && event.GetActionTarget() && event.GetDamageInfo() &&
            event.GetDamageInfo()->GetDamage() > 0 && Area52Marksman::Triggers(Effect(event));
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        Unit* target = event.GetActionTarget();
        if (!owner)
            return;
        auto const shot = Effect(event);
        if (shot.Consumes)
            owner->RemoveAurasDueToSpell(Area52Marksman::CompoundFlurry);
        if (shot.ComboPoints && target)
            owner->AddComboPoints(target, shot.ComboPoints);
        if (shot.Empowers)
            owner->CastSpell(owner, Area52Marksman::CompoundFlurry, true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_compound_power::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_compound_power::Proc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_area52_compound_power_slice_and_dice : public AuraScript
{
    PrepareAuraScript(spell_area52_compound_power_slice_and_dice);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void RangedHaste(AuraEffect const* /*effect*/, int32& amount, bool& /*canBeRecalculated*/)
    {
        Unit* owner = GetUnitOwner();
        if (!owner || !owner->HasAura(Area52Marksman::CompoundPower))
            return;
        if (AuraEffect const* melee = GetAura()->GetEffect(EFFECT_1))
            amount = melee->GetAmount();
        else
            amount = GetSpellInfo()->Effects[EFFECT_1].CalcValue(owner);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_area52_compound_power_slice_and_dice::RangedHaste, EFFECT_2,
            SPELL_AURA_MOD_RANGED_HASTE);
    }
};

class spell_area52_sniper : public AuraScript
{
    PrepareAuraScript(spell_area52_sniper);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Marksman::SniperStack});
    }

    void Tick(AuraEffect const* /*effect*/)
    {
        Unit* owner = GetTarget();
        if (!owner)
            return;
        auto const step = Area52Marksman::SniperTick(_still, owner->isMoving());
        _still = step.Still;
        if (step.Clear)
        {
            owner->RemoveAurasDueToSpell(Area52Marksman::SniperStack);
            return;
        }
        if (Aura* stacks = owner->GetAura(Area52Marksman::SniperStack))
            stacks->RefreshDuration();
        if (step.AddStack)
            owner->CastSpell(owner, Area52Marksman::SniperStack, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_area52_sniper::Tick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }

    std::uint32_t _still = 0;
};
