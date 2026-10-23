/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

class aura_area52_everliving : public AuraScript
{
    PrepareAuraScript(aura_area52_everliving);

    bool Check(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        HealInfo* heal = event.GetHealInfo();
        return player && AscensionMysticEnchant::Applies(player) && event.GetActor() == player &&
            heal && heal->GetEffectiveHeal() > 0 && (heal->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE) &&
            event.GetProcTarget() && player->IsValidAssistTarget(event.GetProcTarget());
    }

    void Handle(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), 415013, true);
    }

    void Cleanup(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !AscensionMysticEnchant::Applies(player))
            return;
        player->RemoveAurasDueToSpell(415013, player->GetGUID());
        player->RemoveAurasDueToSpell(415014, player->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_everliving::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_everliving::Handle,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_everliving::Cleanup,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_everliving_swiftmend : public SpellScript
{
    PrepareSpellScript(spell_area52_everliving_swiftmend);

    uint32 stacks = 0;

    bool Load() override
    {
        Player* player = GetCaster()->ToPlayer();
        return player && AscensionMysticEnchant::Applies(player);
    }

    void Snapshot()
    {
        Unit* caster = GetCaster();
        if (!caster->HasAura(415012))
            return;
        if (Aura* aura = caster->GetAura(415013, caster->GetGUID()))
            stacks = std::min<uint32>(15, aura->GetStackAmount());
        caster->RemoveAurasDueToSpell(415014, caster->GetGUID());
        if (stacks == 15)
            caster->CastSpell(caster, 415014, true);
    }

    void Heal()
    {
        if (!stacks || !GetHitUnit() || !GetHitHeal())
            return;
        SetHitHeal(GetHitHeal() + CalculatePct(GetHitHeal(), stacks * 3));
        GetCaster()->RemoveAurasDueToSpell(415013, GetCaster()->GetGUID());
        GetCaster()->RemoveAurasDueToSpell(415014, GetCaster()->GetGUID());
    }

    void Finish()
    {
        GetCaster()->RemoveAurasDueToSpell(415014, GetCaster()->GetGUID());
    }

    void Register() override
    {
        OnCast += SpellCastFn(spell_area52_everliving_swiftmend::Snapshot);
        OnHit += SpellHitFn(spell_area52_everliving_swiftmend::Heal);
        AfterCast += SpellCastFn(spell_area52_everliving_swiftmend::Finish);
    }
};

class aura_area52_everliving_stacks : public AuraScript
{
    PrepareAuraScript(aura_area52_everliving_stacks);

    void Modifier(AuraEffect const*, SpellModifier*& modifier)
    {
        Player* player = GetTarget()->ToPlayer();
        if (player && AscensionMysticEnchant::Applies(player) && modifier)
            modifier->value = 0;
    }

    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(aura_area52_everliving_stacks::Modifier,
            EFFECT_2, SPELL_AURA_ADD_PCT_MODIFIER);
    }
};
