bool HasArea52MasterOfShadowsHandlers(uint32 spell)
{
    using namespace Area52MasterOfShadows;
    if (spell == MasterOfShadows)
        return sSpellMgr->GetSpellProcEntry(MasterOfShadows) && sSpellMgr->GetSpellInfo(DarkIntent) &&
            sSpellMgr->GetSpellInfo(DarkIntentNoCrit) && sSpellMgr->GetSpellInfo(MasterOfShadowsBuff) &&
            sSpellMgr->GetSpellInfo(MasterOfShadowsCritical) &&
            HasAuraScriptNamed(MasterOfShadows, "spell_area52_master_of_shadows") &&
            HasAuraScriptNamed(ApprenticeOfShadows, "spell_area52_apprentice_of_shadows");
    if (spell == Pestilence)
        return sSpellMgr->GetSpellProcEntry(Pestilence) && HasAuraScriptNamed(Pestilence, "spell_area52_pestilence");
    if (spell == ShadowStrikes)
        return sSpellMgr->GetSpellProcEntry(ShadowStrikes) && sSpellMgr->GetSpellInfo(CruelNight);
    if (spell == FindWeakness)
        return HasAuraScriptNamed(FindWeakness, "spell_area52_find_weakness");
    return false;
}

Area52MasterOfShadows::SpellKey Area52ShadowsSpell(SpellInfo const* info)
{
    return {info->Id, info->SpellFamilyName, info->SpellFamilyFlags[0]};
}

void Area52StrikeDarkIntent(Unit* caster, Unit* target, bool critical, AuraEffect const* effect)
{
    using namespace Area52MasterOfShadows;
    if (critical)
        caster->CastCustomSpell(DarkIntent, SPELLVALUE_FORCED_CRIT_RESULT, 1, target, true, nullptr, effect);
    else
        caster->CastSpell(target, DarkIntentNoCrit, true, nullptr, effect);
}

class spell_area52_master_of_shadows : public AuraScript
{
    PrepareAuraScript(spell_area52_master_of_shadows);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        using namespace Area52MasterOfShadows;
        return ValidateSpellInfo({DarkIntent, DarkIntentNoCrit, ApprenticeOfShadows, MasterOfShadowsBuff,
            MasterOfShadowsCritical});
    }

    void HandleProc(AuraEffect const* effect, ProcEventInfo& event)
    {
        using namespace Area52MasterOfShadows;
        PreventDefaultAction();
        Unit* rogue = GetTarget();
        SpellInfo const* info = event.GetSpellInfo();
        if (!info)
            return;
        SpellKey const spell = Area52ShadowsSpell(info);
        bool const periodic = event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC;
        bool const critical = event.GetHitMask() & PROC_HIT_CRITICAL;
        Unit* target = event.GetActionTarget();
        DamageInfo* damage = event.GetDamageInfo();
        if (target && target != rogue && damage && damage->GetDamage() && TriggersDarkIntent(spell, periodic))
            Area52StrikeDarkIntent(rogue, target, critical, effect);
        if (GrantsApprentice(spell, periodic, critical))
            rogue->CastSpell(rogue, ApprenticeOfShadows, true, nullptr, effect);
        Aura* apprentice = rogue->GetAura(ApprenticeOfShadows);
        if (!apprentice || !SpendsApprentice(spell, periodic, apprentice->GetStackAmount()))
            return;
        rogue->RemoveAura(apprentice);
        rogue->CastSpell(rogue, MasterOfShadowsBuff, true, nullptr, effect);
        rogue->CastSpell(rogue, MasterOfShadowsCritical, true, nullptr, effect);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_area52_master_of_shadows::HandleProc, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_apprentice_of_shadows : public AuraScript
{
    PrepareAuraScript(spell_area52_apprentice_of_shadows);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool SpentByMasterOfShadows(ProcEventInfo&)
    {
        return false;
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_apprentice_of_shadows::SpentByMasterOfShadows);
    }
};

class spell_area52_pestilence : public AuraScript
{
    PrepareAuraScript(spell_area52_pestilence);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    static bool Excluded(Unit* caster, Unit* source, Unit* victim)
    {
        return victim == source || victim->IsTotem() || !caster->IsValidAttackTarget(victim) ||
            (victim->IsPlayer() && victim->HasBreakableByDamageCrowdControlAura());
    }

    static void Spread(Unit* caster, Unit* source, Aura const* corruption)
    {
        using namespace Area52MasterOfShadows;
        std::list<Unit*> candidates;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(source, caster, PestilenceRadius);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(source, candidates, check);
        Cell::VisitObjects(source, searcher, PestilenceRadius);
        std::size_t spread = 0;
        for (Unit* victim : candidates)
        {
            if (spread >= PestilenceMaxTargets)
                return;
            if (Excluded(caster, source, victim))
                continue;
            Aura* copy = victim->GetAura(corruption->GetId(), caster->GetGUID());
            if (!ShouldCopyRemaining(corruption->GetDuration(), copy ? copy->GetDuration() : 0, copy != nullptr))
                continue;
            if (!copy)
                copy = caster->AddAura(corruption->GetId(), victim);
            if (!copy)
                continue;
            copy->SetMaxDuration(corruption->GetMaxDuration());
            copy->SetDuration(corruption->GetDuration());
            ++spread;
        }
    }

    void SpreadCorruption(AuraEffect const*, ProcEventInfo& event)
    {
        using namespace Area52MasterOfShadows;
        PreventDefaultAction();
        Unit* caster = GetTarget();
        Unit* source = event.GetActionTarget();
        if (!source || source == caster)
            return;
        std::vector<Aura const*> corruptions;
        for (AuraEffect const* periodic : source->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DAMAGE))
        {
            SpellInfo const* info = periodic->GetSpellInfo();
            if (periodic->GetCasterGUID() == caster->GetGUID() && info->SpellFamilyName == SPELLFAMILY_WARLOCK &&
                (info->SpellFamilyFlags[0] & WarlockCorruptionFlag0))
                corruptions.push_back(periodic->GetBase());
        }
        for (Aura const* corruption : corruptions)
            Spread(caster, source, corruption);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_area52_pestilence::SpreadCorruption, EFFECT_1,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_find_weakness : public AuraScript
{
    PrepareAuraScript(spell_area52_find_weakness);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void AllAbilityDots(AuraEffect const*, SpellModifier*& mod)
    {
        if (!mod)
            return;
        SpellEffectInfo const& abilities = GetSpellInfo()->Effects[EFFECT_0];
        mod->mask = abilities.SpellClassMask | GetSpellInfo()->Effects[EFFECT_1].SpellClassMask;
        mod->value = abilities.CalcValue();
    }

    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_area52_find_weakness::AllAbilityDots, EFFECT_1,
            SPELL_AURA_ADD_PCT_MODIFIER);
    }
};

class area52_master_of_shadows_casts : public AllSpellScript
{
public:
    area52_master_of_shadows_casts() : AllSpellScript("area52_master_of_shadows_casts",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CALCULATED_TARGET}) { }

    static bool ConsumesShadowTrance(Spell const* spell)
    {
        for (Aura const* aura : spell->m_appliedMods)
            if (aura->GetId() == Area52MasterOfShadows::ShadowTrance)
                return true;
        return false;
    }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        using namespace Area52MasterOfShadows;
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || spell->IsTriggered() || !AscensionFreePick::Applies(player) || !ConsumesShadowTrance(spell))
            return;
        if (player->HasAura(ShadowStrikes))
            player->CastSpell(player, CruelNight, true);
        Unit* target = spell->m_targets.GetUnitTarget();
        AuraEffect const* master = player->GetAuraEffect(MasterOfShadows, EFFECT_0);
        if (!master || !target || target == player || !IsShadowBolt(Area52ShadowsSpell(info)))
            return;
        for (uint8 hit = 0; hit < NightfallDarkIntentHits; ++hit)
            Area52StrikeDarkIntent(player, target, true, master);
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit*, TargetInfo& hit) override
    {
        if (spell->GetSpellInfo()->Id == Area52MasterOfShadows::DarkIntent && spell->GetSpellValue()->ForcedCritResult)
            hit.crit = true;
    }
};
