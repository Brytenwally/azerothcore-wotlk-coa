bool HasArea52GutripperHandlers(uint32 spell)
{
    using namespace Area52Gutripper;
    if (spell == BloodAndGuts)
        return sSpellMgr->GetSpellProcEntry(BloodAndGuts) && sSpellMgr->GetSpellProcEntry(ExecutingDisembowelAura) &&
            sSpellMgr->GetSpellInfo(ExecutingDisembowel) &&
            HasAuraScriptNamed(BloodAndGuts, "spell_area52_blood_and_guts") &&
            HasAuraScriptNamed(ExecutingDisembowelAura, "spell_area52_executing_disembowel") &&
            HasAuraScriptNamed(Disembowel, "spell_area52_disembowel") &&
            HasSpellScriptNamed(ExecutingDisembowel, "spell_area52_executing_disembowel_burst");
    if (spell == DeadlyCombo)
        return sSpellMgr->GetSpellInfo(SuddenDeathProc) && sSpellMgr->GetSpellInfo(DeadlyComboBuff) &&
            HasSpellScriptNamed(ExecuteDamage, "spell_area52_gutripper_execute");
    return false;
}

bool DisemboweledBy(Unit const* target, Unit const* rogue)
{
    return target && rogue && target->GetAuraOfRankedSpell(Area52Gutripper::Disembowel, rogue->GetGUID());
}

class spell_area52_disembowel : public AuraScript
{
    PrepareAuraScript(spell_area52_disembowel);

    void AddPower(AuraEffect const*, int32& amount, bool&)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        float const power = player->GetTotalAttackPowerValue(BASE_ATTACK) +
            float(player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NORMAL));
        amount += Area52Gutripper::DisembowelPowerBonus(player->GetComboPoints(), power);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_area52_disembowel::AddPower, EFFECT_0,
            SPELL_AURA_PERIODIC_DAMAGE);
    }
};

class spell_area52_blood_and_guts : public AuraScript
{
    PrepareAuraScript(spell_area52_blood_and_guts);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool OnDisemboweled(ProcEventInfo& event)
    {
        return event.GetActor() == GetTarget() && DisemboweledBy(event.GetActionTarget(), GetTarget());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_blood_and_guts::OnDisemboweled);
    }
};

class spell_area52_executing_disembowel : public AuraScript
{
    PrepareAuraScript(spell_area52_executing_disembowel);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool OnDisemboweled(ProcEventInfo& event)
    {
        return event.GetActor() == GetTarget() && DisemboweledBy(event.GetActionTarget(), GetTarget());
    }

    void ConsumeBleed(AuraEffect const*, ProcEventInfo& event)
    {
        if (Unit* target = event.GetActionTarget())
            if (Aura* bleed = target->GetAuraOfRankedSpell(Area52Gutripper::Disembowel, GetTarget()->GetGUID()))
                bleed->Remove();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_executing_disembowel::OnDisemboweled);
        AfterEffectProc += AuraEffectProcFn(spell_area52_executing_disembowel::ConsumeBleed, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_executing_disembowel_burst : public SpellScript
{
    PrepareSpellScript(spell_area52_executing_disembowel_burst);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Gutripper::RemorselessBuff, Area52Gutripper::RemorselessCapstoneBuff});
    }

    void SkipRemorseless(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void GrantRemorseless()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;
        if (player->HasSpell(Area52Gutripper::RemorselessAttacksCapstone))
        {
            player->CastSpell(player, Area52Gutripper::RemorselessCapstoneBuff, true);
            if (AuraEffect const* sliceAndDice = player->GetAuraEffect(SPELL_AURA_MOD_MELEE_HASTE, SPELLFAMILY_ROGUE,
                0x40000, 0, 0, player->GetGUID()))
            {
                Aura* aura = sliceAndDice->GetBase();
                aura->SetDuration(sliceAndDice->GetSpellInfo()->GetMaxDuration(), true);
                aura->SetMaxDuration(aura->GetDuration());
            }
        }
        else if (player->HasSpell(Area52Gutripper::RemorselessAttacks))
            player->CastSpell(player, Area52Gutripper::RemorselessBuff, true);
    }

    void Register() override
    {
        for (SpellEffIndex effect : {EFFECT_1, EFFECT_2})
        {
            OnEffectLaunch += SpellEffectFn(spell_area52_executing_disembowel_burst::SkipRemorseless, effect,
                SPELL_EFFECT_TRIGGER_SPELL);
            OnEffectLaunchTarget += SpellEffectFn(spell_area52_executing_disembowel_burst::SkipRemorseless, effect,
                SPELL_EFFECT_TRIGGER_SPELL);
        }
        AfterCast += SpellCastFn(spell_area52_executing_disembowel_burst::GrantRemorseless);
    }
};

class spell_area52_gutripper_execute : public SpellScript
{
    PrepareSpellScript(spell_area52_gutripper_execute);

    void AddJudgedBonus()
    {
        Unit* warrior = GetCaster();
        Unit* target = GetHitUnit();
        if (!warrior || !target || !AscensionFreePick::Applies(warrior->ToPlayer()))
            return;
        int32 bonus = 0;
        if (AuraEffect const* sentence = warrior->GetAuraEffect(Area52Gutripper::DeathSentence, EFFECT_0))
            if (target->GetAuraOfRankedSpell(Area52Gutripper::JudgementOfJustice, warrior->GetGUID()))
                bonus += sentence->GetAmount();
        if (AuraEffect const* stillness = warrior->GetAuraEffect(Area52Gutripper::BeStillAndDie, EFFECT_0))
            if (target->GetAuraOfRankedSpell(Area52Gutripper::HammerOfJustice, warrior->GetGUID()))
                bonus += stillness->GetAmount();
        SetHitDamage(Area52Gutripper::WithExecuteBonus(GetHitDamage(), bonus));
    }

    void SpendNextExecuteBuffs()
    {
        Unit* warrior = GetCaster();
        if (!warrior || !AscensionFreePick::Applies(warrior->ToPlayer()))
            return;
        warrior->RemoveAurasDueToSpell(Area52Gutripper::DeadlyComboBuff);
        warrior->RemoveAurasDueToSpell(Area52Gutripper::JudgeJuryExecutionerBuff);
        if (Unit* target = GetHitUnit())
            target->RemoveAurasDueToSpell(Area52Gutripper::Weakness, warrior->GetGUID());
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_gutripper_execute::AddJudgedBonus);
        AfterHit += SpellHitFn(spell_area52_gutripper_execute::SpendNextExecuteBuffs);
    }
};

class spell_area52_sudden_death_cooldown : public AuraScript
{
    PrepareAuraScript(spell_area52_sudden_death_cooldown);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Gutripper::SuddenDeathCooldown});
    }

    bool OffCooldown(ProcEventInfo& event)
    {
        return event.GetSpellPhaseMask() == PROC_SPELL_PHASE_FINISH ||
            !GetTarget()->HasAura(Area52Gutripper::SuddenDeathCooldown);
    }

    void StartCooldown(AuraEffect const*, ProcEventInfo& event)
    {
        if (event.GetSpellPhaseMask() != PROC_SPELL_PHASE_FINISH)
            GetTarget()->CastSpell(GetTarget(), Area52Gutripper::SuddenDeathCooldown, true);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_sudden_death_cooldown::OffCooldown);
        AfterEffectProc += AuraEffectProcFn(spell_area52_sudden_death_cooldown::StartCooldown, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_necrotic_execute : public AuraScript
{
    PrepareAuraScript(spell_area52_necrotic_execute);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Gutripper::NecroticExecuteHelper, Area52Gutripper::NecroticPower,
            Area52Gutripper::WorldforgedCooldown});
    }

    bool Killed(ProcEventInfo& event)
    {
        return (event.GetTypeMask() & PROC_FLAG_KILL) && !GetTarget()->HasAura(Area52Gutripper::WorldforgedCooldown) &&
            roll_chance_i(sSpellMgr->GetSpellInfo(Area52Gutripper::NecroticExecuteHelper)->ProcChance);
    }

    void GrantPower(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastCustomSpell(Area52Gutripper::NecroticPower, SPELLVALUE_BASE_POINT0, effect->GetAmount(),
            GetTarget(), true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_necrotic_execute::Killed);
        OnEffectProc += AuraEffectProcFn(spell_area52_necrotic_execute::GrantPower, EFFECT_0,
            SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

class area52_deadly_combo : public AllSpellScript
{
public:
    area52_deadly_combo() : AllSpellScript("area52_deadly_combo", {ALLSPELLHOOK_ON_CAST}) { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || spell->IsTriggered() || !info->NeedsComboPoints() ||
            !player->HasAura(Area52Gutripper::DeadlyCombo) || !player->HasSpell(Area52Gutripper::SuddenDeathMaxRank) ||
            !AscensionFreePick::Applies(player))
            return;
        if (!roll_chance_i(Area52Gutripper::DeadlyComboChance(player->GetComboPoints())))
            return;
        player->CastSpell(player, Area52Gutripper::SuddenDeathProc, true);
        player->CastSpell(player, Area52Gutripper::DeadlyComboBuff, true);
    }
};
