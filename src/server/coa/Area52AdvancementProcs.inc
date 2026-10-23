class spell_area52_advancement_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_advancement_proc);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({912753, 12309, 901118, 814142, 814769, 816969, 831682, 833228});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        DamageInfo const* damage = event.GetDamageInfo();
        Unit* target = event.GetActionTarget();
        Area52AdvancementProcs::Event facts;
        facts.FreePick = player && AscensionFreePick::Applies(player);
        facts.OwnerIsActor = event.GetActor() == GetTarget();
        facts.Damage = damage && damage->GetDamage() > 0;
        facts.Periodic = (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0;
        facts.Dodged = (event.GetHitMask() & PROC_HIT_DODGE) != 0;
        facts.Behind = target && !target->HasInArc(float(M_PI), GetTarget());
        facts.SpellRoot = spell ? spell->GetFirstRankSpell()->Id : 0;
        facts.Family = spell ? spell->SpellFamilyName : 0;
        facts.School = spell ? spell->SchoolMask : 0;
        return Area52AdvancementProcs::Accept(GetId(), facts);
    }

    void HandleProc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        uint32 const trigger = GetSpellInfo()->Effects[effect->GetEffIndex()].TriggerSpell;
        if (GetId() == 31682 || GetId() == 31683 || GetId() == 31684)
        {
            int32 critical = GetEffect(EFFECT_0)->GetAmount();
            int32 frost = GetEffect(EFFECT_1)->GetAmount();
            owner->CastCustomSpell(owner, trigger, &critical, &frost, nullptr, true, nullptr, effect);
            return;
        }
        bool const self = GetId() == 12753 || GetId() == 14142;
        Unit* target = self ? owner : event.GetActionTarget();
        if (target && trigger)
            owner->CastSpell(target, trigger, true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_advancement_proc::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_advancement_proc::HandleProc, EFFECT_ALL, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};
