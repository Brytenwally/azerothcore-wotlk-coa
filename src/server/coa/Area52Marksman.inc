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

bool HasArea52MarksmanHandlers(uint32 spell)
{
    if (spell == Area52Marksman::CompoundPower)
        return sSpellMgr->GetSpellProcEntry(spell) && sSpellMgr->GetSpellInfo(Area52Marksman::CompoundFlurry) &&
            HasAuraScriptNamed(spell, "spell_area52_compound_power") &&
            HasAuraScriptNamed(Area52Marksman::SliceAndDice, "spell_area52_compound_power_slice_and_dice") &&
            sSpellMgr->GetSpellInfo(Area52Marksman::ComboShot) &&
            HasSpellScriptNamed(Area52Marksman::CompoundShot, "spell_area52_compound_shot") &&
            HasAuraScriptNamed(Area52Marksman::CompoundShotRecast, "aura_area52_compound_shot_recast") &&
            HasSpellScriptNamed(Area52Marksman::ComboShotVolley, "spell_area52_combo_shot_volley");
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

class spell_area52_compound_shot : public SpellScript
{
    PrepareSpellScript(spell_area52_compound_shot);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Marksman::CompoundShotRecast});
    }

    void OpenRecastWindow()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        player->CastSpell(player, Area52Marksman::CompoundShotRecast, true);
        if (Aura* recast = player->GetAura(Area52Marksman::CompoundShotRecast))
            recast->SetDuration(Area52Marksman::RecastWindowMs);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_compound_shot::OpenRecastWindow);
    }
};

class aura_area52_compound_shot_recast : public AuraScript
{
    PrepareAuraScript(aura_area52_compound_shot_recast);

    static void Swap(Unit* target, bool toComboShot)
    {
        Player* player = target ? target->ToPlayer() : nullptr;
        if (!player)
            return;
        std::set<uint32> visited;
        for (uint32 rank = Area52Marksman::CompoundShot; rank && visited.insert(rank).second;
            rank = sSpellMgr->GetNextSpellInChain(rank))
            player->SetTemporarySpellReplacement(rank,
                toComboShot && player->HasActiveSpell(rank) ? Area52Marksman::ComboShot : 0);
    }

    void Open(AuraEffect const*, AuraEffectHandleModes)
    {
        Swap(GetTarget(), true);
    }

    void Close(AuraEffect const*, AuraEffectHandleModes)
    {
        Swap(GetTarget(), false);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_compound_shot_recast::Open, EFFECT_1,
            SPELL_AURA_ABILITY_IGNORE_AURASTATE, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_compound_shot_recast::Close, EFFECT_1,
            SPELL_AURA_ABILITY_IGNORE_AURASTATE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_combo_shot_volley : public SpellScript
{
    PrepareSpellScript(spell_area52_combo_shot_volley);

    void KeepFighting(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            return !unit || !unit->IsInCombat();
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_area52_combo_shot_volley::KeepFighting,
            EFFECT_ALL, TARGET_UNIT_CONE_ENEMY_104);
    }
};

class spell_area52_syndicate_sniper_training : public AuraScript
{
    PrepareAuraScript(spell_area52_syndicate_sniper_training);

    void Mirror(AuraEffect const*)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !AscensionFreePick::Applies(player))
            return;
        uint32 const training = Area52Marksman::SyndicateTrainingFor(GetId());
        Aura const* buff = player->GetAura(Area52Marksman::SniperTrainingBuffFor(GetId()));
        if (!buff || !player->HasAura(Area52Marksman::SyndicateSniperShotEnchant))
        {
            player->RemoveAurasDueToSpell(training);
            return;
        }
        Aura* mirror = player->GetAura(training);
        if (!mirror)
        {
            player->CastSpell(player, training, true);
            mirror = player->GetAura(training);
        }
        if (!mirror)
            return;
        if (mirror->GetStackAmount() != buff->GetStackAmount())
            mirror->SetStackAmount(buff->GetStackAmount());
        mirror->SetDuration(buff->GetDuration());
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_area52_syndicate_sniper_training::Mirror, EFFECT_0,
            SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_area52_syndicate_capstone : public AuraScript
{
    PrepareAuraScript(spell_area52_syndicate_capstone);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Marksman::HoneIn, Area52Marksman::HoneInDamage});
    }

    bool Ready(ProcEventInfo& event)
    {
        Unit* target = event.GetActionTarget();
        return target && AscensionFreePick::Applies(GetTarget()->ToPlayer()) &&
            Area52Marksman::HonesIn(GetStackAmount(), target->IsControlledByPlayer());
    }

    void HoneIn(AuraEffect const*, ProcEventInfo&)
    {
        Unit* hunter = GetTarget();
        hunter->CastSpell(hunter, Area52Marksman::HoneIn, true);
        hunter->CastSpell(hunter, Area52Marksman::HoneInDamage, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_syndicate_capstone::Ready);
        OnEffectProc += AuraEffectProcFn(spell_area52_syndicate_capstone::HoneIn, EFFECT_2, SPELL_AURA_DUMMY);
    }
};

class spell_area52_syndicate_sniper_shot : public SpellScript
{
    PrepareSpellScript(spell_area52_syndicate_sniper_shot);

    void SpendHoneIn()
    {
        if (Unit* hunter = GetCaster())
        {
            hunter->RemoveAurasDueToSpell(Area52Marksman::HoneIn);
            hunter->RemoveAurasDueToSpell(Area52Marksman::HoneInDamage);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_syndicate_sniper_shot::SpendHoneIn);
    }
};
