class spell_area52_corrupted_bear_instant : public AuraScript
{
    PrepareAuraScript(spell_area52_corrupted_bear_instant);

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Area52CorruptedBear::Synchronize(GetTarget());
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetTarget()->HasAura(GetId() == 17941 ? 34936 : 17941))
            Area52CorruptedBear::Synchronize(GetTarget());
        else if (AscensionFreePick::Applies(GetTarget()->ToPlayer()))
            GetTarget()->RemoveAurasDueToSpell(300600, GetTarget()->GetGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_corrupted_bear_instant::Apply,
            EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_corrupted_bear_instant::Remove,
            EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }
};

class area52_corrupted_bear_casts : public AllSpellScript
{
public:
    area52_corrupted_bear_casts() : AllSpellScript("area52_corrupted_bear_casts",
        {ALLSPELLHOOK_ON_PREPARE, ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CALCULATED_TARGET}) { }

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        if (spell && !spell->IsTriggered() && info->GetFirstRankSpell()->Id == 686 &&
            Area52CorruptedBear::Active(caster) && (caster->HasAura(17941) || caster->HasAura(34936)))
            spell->SetScriptValue(99634, 1);
    }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const*) override
    {
        if (!spell || !spell->GetScriptValue(99634) || !Area52CorruptedBear::Active(caster))
            return;
        Unit* primary = spell->m_targets.GetUnitTarget();
        if (!primary)
            return;
        std::list<Unit*> candidates;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(primary, caster, 10.0f);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(primary, candidates, check);
        Cell::VisitObjects(primary, searcher, 10.0f);
        candidates.sort([primary](Unit* left, Unit* right)
            { return primary->GetDistance(left) < primary->GetDistance(right); });
        uint32 remaining = 2;
        for (Unit* candidate : candidates)
            if (candidate != primary && candidate->IsAlive() && caster->IsValidAttackTarget(candidate) &&
                primary->IsWithinLOSInMap(candidate) && caster->IsWithinLOSInMap(candidate))
            {
                spell->AddUnitTargetForScript(candidate, 1);
                if (!--remaining)
                    break;
            }
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit*, TargetInfo& hit) override
    {
        if (spell && spell->GetScriptValue(99634) && hit.damage > 0)
            if (SpellInfo const* reduction = sSpellMgr->GetSpellInfo(986146))
                hit.damage = int32(int64(hit.damage) *
                    std::clamp(100 + reduction->Effects[EFFECT_0].CalcValue(spell->GetCaster()), 0, 100) / 100);
    }
};
