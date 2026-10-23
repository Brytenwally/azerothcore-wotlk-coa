#include "Area52DependencyRules.h"

class spell_area52_dependency_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_dependency_proc);

    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({17941, 48108, 74396, 978425, 48391});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        auto const* spell = event.GetSpellInfo();
        uint32 const root = spell ? spell->GetFirstRankSpell()->Id : 0;
        switch (GetSpellInfo()->GetFirstRankSpell()->Id)
        {
            case 18094:
                return spell && Area52Dependency::NightfallSource(root);
            case 44445:
                return spell && !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) &&
                    (Area52Dependency::HotStreakSource(root) || (root == 6353 && GetTarget()->HasAura(414080)));
            case 44543:
                return spell && Area52Dependency::FingersSource(root, spell->SpellFamilyName,
                    event.GetSchoolMask(), event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC);
            case 48389:
                return GetTarget()->GetShapeshiftForm() == FORM_MOONKIN && event.GetDamageInfo() &&
                    event.GetDamageInfo()->GetDamage() && event.GetActor() != GetTarget();
            default:
                return false;
        }
    }

    void Handle(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        switch (GetSpellInfo()->GetFirstRankSpell()->Id)
        {
            case 18094:
                owner->CastCustomSpell(17941, SPELLVALUE_BASE_POINT1,
                    -25 * std::min<uint8>(2, GetSpellInfo()->GetRank()), owner, true, nullptr, effect);
                break;
            case 44445:
                if (Area52Dependency::CompleteCriticalPair(Streak, event.GetHitMask() & PROC_HIT_CRITICAL) &&
                    roll_chance_i(effect->GetAmount()))
                    owner->CastSpell(owner, 48108, true, nullptr, effect);
                break;
            case 44543:
                owner->CastSpell(owner, 74396, true, nullptr, effect);
                if (owner->HasAura(978208))
                    owner->CastSpell(owner, 978425, true, nullptr, effect);
                break;
            case 48389:
                owner->CastSpell(owner, 48391, true, nullptr, effect);
                break;
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_dependency_proc::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_dependency_proc::Handle, EFFECT_0, SPELL_AURA_ANY);
    }

    uint8 Streak = 0;
};

class area52_dependency_metadata : public GlobalScript
{
public:
    area52_dependency_metadata() : GlobalScript("area52_dependency_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        switch (info->Id)
        {
            case 828574:
            case 828575:
            case 828576:
                info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_DUMMY;
                break;
            case 828590:
            case 828591:
            case 828594:
            case 828595:
            case 828596:
            case 828597:
                info->SpellFamilyName = SPELLFAMILY_GENERIC;
                info->AttributesCu |= SPELL_ATTR0_CU_SHARE_DAMAGE;
                break;
            case 34935:
            case 34938:
            case 34939:
                info->Effects[EFFECT_2].Effect = SPELL_EFFECT_APPLY_AURA;
                info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_MOD_WEAPON_CRIT_PERCENT;
                info->Effects[EFFECT_2].BasePoints = info->Effects[EFFECT_1].BasePoints;
                info->Effects[EFFECT_2].DieSides = 1;
                info->Effects[EFFECT_2].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                break;
            case 34936:
                info->Effects[EFFECT_0].SpellClassMask[1] |= 64;
                info->Effects[EFFECT_1].SpellClassMask[1] |= 64;
                break;
            case 48108:
                info->Effects[EFFECT_2].ApplyAuraName = SPELL_AURA_ADD_PCT_MODIFIER;
                info->Effects[EFFECT_2].MiscValue = SPELLMOD_COST;
                info->Effects[EFFECT_2].BasePoints = -51;
                info->Effects[EFFECT_2].DieSides = 1;
                break;
            case 48391:
                info->Attributes &= ~SPELL_ATTR0_NOT_SHAPESHIFTED;
                break;
            case 701464:
                info->Effects[EFFECT_0].BasePoints = -31;
                info->Effects[EFFECT_1].BasePoints = -41;
                info->Effects[EFFECT_0].RealPointsPerLevel = 0;
                info->Effects[EFFECT_1].RealPointsPerLevel = 0;
                info->Effects[EFFECT_2].Effect = 0;
                break;
            case 701467:
                info->Effects[EFFECT_0].Effect = 0;
                info->Effects[EFFECT_1].Effect = 0;
                break;
            case 701463:
                info->Effects[EFFECT_1].Effect = SPELL_EFFECT_APPLY_AURA;
                info->Effects[EFFECT_1].ApplyAuraName = SPELL_AURA_DUMMY;
                info->Effects[EFFECT_1].TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
                break;
            case 277432:
                info->AttributesEx5 &= ~SPELL_ATTR5_EXTRA_INITIAL_PERIOD;
                break;
            case 954876:
            case 955126:
            case 955127:
            case 955128:
            case 955129:
                info->MaxAffectedTargets = 5;
                break;
            case 86361:
            case 93924:
            case 93925:
            case 93926:
            case 93927:
            case 93928:
                info->AttributesCu |= SPELL_ATTR0_CU_IGNORE_ARMOR;
                info->Effects[EFFECT_1].SpellClassMask = flag96();
                break;
            default:
                break;
        }
    }
};

class spell_area52_mana_barrier : public AuraScript
{
    PrepareAuraScript(spell_area52_mana_barrier);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({78102, 701464, 701467, 277432}); }

    void Amount(AuraEffect const*, int32& amount, bool& recalculate)
    {
        amount = -1;
        recalculate = false;
    }

    void Absorb(AuraEffect*, DamageInfo& damage, uint32& amount)
    {
        Unit* owner = GetTarget();
        amount = Area52Dependency::ManaBarrierAbsorb(damage.GetDamage(), owner->GetPower(POWER_MANA),
            owner->GetMaxPower(POWER_MANA), damage.GetSchoolMask() & SPELL_SCHOOL_MASK_MAGIC);
    }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        for (uint32 spell : {78102u, 701464u, 701467u})
            GetTarget()->CastSpell(GetTarget(), spell, true);
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        for (uint32 spell : {78102u, 701464u, 701467u, 277432u})
            GetTarget()->RemoveAurasDueToSpell(spell, GetTarget()->GetGUID());
    }

    bool CheckProc(ProcEventInfo& event)
    {
        if (!event.GetDamageInfo() || event.GetActor() == GetTarget() ||
            GetTarget()->GetPower(POWER_MANA) >= GetTarget()->GetMaxPower(POWER_MANA))
            return false;
        Spell* cast = GetTarget()->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (!cast)
            cast = GetTarget()->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!cast)
            return false;
        auto const* info = cast->GetSpellInfo();
        return info->IsChanneled() || info->HasEffect(SPELL_EFFECT_SCHOOL_DAMAGE) ||
            info->HasEffect(SPELL_EFFECT_HEALTH_LEECH) || info->HasAura(SPELL_AURA_PERIODIC_DAMAGE) ||
            info->HasAura(SPELL_AURA_PERIODIC_LEECH);
    }

    void Surge(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        int32 const mana = int32(Area52Dependency::MissingManaSurge(GetTarget()->GetPower(POWER_MANA),
            GetTarget()->GetMaxPower(POWER_MANA)));
        GetTarget()->CastCustomSpell(277432, SPELLVALUE_BASE_POINT0, mana, GetTarget(), true, nullptr, effect);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_area52_mana_barrier::Amount, EFFECT_0, SPELL_AURA_MANA_SHIELD);
        OnEffectManaShield += AuraEffectManaShieldFn(spell_area52_mana_barrier::Absorb, EFFECT_0);
        AfterEffectApply += AuraEffectApplyFn(spell_area52_mana_barrier::Apply,
            EFFECT_0, SPELL_AURA_MANA_SHIELD, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_mana_barrier::Remove,
            EFFECT_0, SPELL_AURA_MANA_SHIELD, AURA_EFFECT_HANDLE_REAL);
        DoCheckProc += AuraCheckProcFn(spell_area52_mana_barrier::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_mana_barrier::Surge, EFFECT_1, SPELL_AURA_DUMMY);
    }
};

class aura_area52_missing_mana_surge : public AuraScript
{
    PrepareAuraScript(aura_area52_missing_mana_surge);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({277430}); }

    void Apply(AuraEffect const* effect, AuraEffectHandleModes)
    {
        Remaining += std::max(0, effect->GetAmount());
        Ticks = 4;
    }

    void Tick(AuraEffect const*)
    {
        PreventDefaultAction();
        if (!Ticks)
            return;
        uint32 const amount = Remaining / Ticks;
        Remaining -= amount;
        --Ticks;
        GetTarget()->EnergizeBySpell(GetTarget(), 277430, amount, POWER_MANA);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_missing_mana_surge::Apply,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, AURA_EFFECT_HANDLE_REAL_OR_REAPPLY_MASK);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_missing_mana_surge::Tick,
            EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    }

    uint32 Remaining = 0;
    uint32 Ticks = 0;
};

class area52_dependency_damage : public UnitScript
{
public:
    area52_dependency_damage() : UnitScript("area52_dependency_damage", true,
        {UNITHOOK_MODIFY_MELEE_DAMAGE, UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
         UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK}) { }

    uint32 Percent(Unit* target, Unit* attacker) const
    {
        if (!target || !attacker || !AscensionFreePick::Applies(target->ToPlayer()))
            return 100;
        return Area52Dependency::PlayerDamageTakenPercent(attacker->GetCharmerOrOwnerPlayerOrPlayerItself() != nullptr,
            target->HasAura(701463), target->GetShapeshiftForm() == FORM_BEAR ||
            target->GetShapeshiftForm() == FORM_DIREBEAR);
    }

    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        damage = uint32(uint64(damage) * Percent(target, attacker) / 100);
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const*) override
    {
        if (damage > 0)
            damage = int32(int64(damage) * Percent(target, attacker) / 100);
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* info) override
    {
        if (info && !info->IsPositive() &&
            (info->HasAura(SPELL_AURA_PERIODIC_DAMAGE) || info->HasAura(SPELL_AURA_PERIODIC_DAMAGE_PERCENT) ||
             info->HasAura(SPELL_AURA_PERIODIC_LEECH)))
            damage = uint32(uint64(damage) * Percent(target, attacker) / 100);
        if (info && attacker && target && info->GetFirstRankSpell()->Id == 11366 &&
            AscensionFreePick::Applies(attacker->ToPlayer()) && !target->GetCharmerOrOwnerPlayerOrPlayerItself())
            if (Aura* aura = target->GetAura(info->Id, attacker->GetGUID()))
                if (aura->GetScriptValue(44445))
                    damage = uint32(uint64(damage) * 140 / 100);
    }
};

class area52_dependency_casts : public AllSpellScript
{
public:
    area52_dependency_casts() : AllSpellScript("area52_dependency_casts",
        {ALLSPELLHOOK_ON_PREPARE, ALLSPELLHOOK_ON_CALCULATED_TARGET, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        if (!spell || spell->IsTriggered() || !AscensionFreePick::Applies(caster->ToPlayer()))
            return;
        uint32 const root = info->GetFirstRankSpell()->Id;
        if (root == 686 || root == 29722 || root == 11366)
        {
            spell->SetScriptValue(18094, caster->HasAura(17941));
            spell->SetScriptValue(34935, caster->HasAura(34936));
            spell->SetScriptValue(44445, caster->HasAura(48108));
        }
    }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
    {
        if (!spell || !target || hit.damage <= 0 || !AscensionFreePick::Applies(spell->GetCaster()->ToPlayer()))
            return;
        auto const percent = Area52Dependency::DamagePercent(spell->GetSpellInfo()->GetFirstRankSpell()->Id,
            target->GetCharmerOrOwnerPlayerOrPlayerItself() != nullptr, spell->GetScriptValue(18094),
            spell->GetScriptValue(34935), spell->GetScriptValue(44445));
        hit.damage = int32(int64(hit.damage) * percent / 100);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32, uint32, bool) override
    {
        if (!spell || !target || miss != SPELL_MISS_NONE ||
            spell->GetSpellInfo()->GetFirstRankSpell()->Id != 11366 ||
            !AscensionFreePick::Applies(spell->GetCaster()->ToPlayer()))
            return;
        if (Aura* aura = target->GetAura(spell->GetSpellInfo()->Id, spell->GetCaster()->GetGUID()))
            aura->SetScriptValue(44445, spell->GetScriptValue(44445));
    }
};

class spell_area52_deep_freeze : public SpellScript
{
    PrepareSpellScript(spell_area52_deep_freeze);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({844572, 71757}); }

    void Hit(SpellEffIndex)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;
        bool const permanent = target->IsCreature() &&
            target->ToCreature()->HasMechanicTemplateImmunity(1ULL << MECHANIC_STUN);
        GetCaster()->CastSpell(target, permanent ? 71757 : 844572, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_deep_freeze::Hit, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_area52_siegebreaker : public SpellScript
{
    PrepareSpellScript(spell_area52_siegebreaker);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({954877}); }

    bool Collision()
    {
        auto const* aura = GetSpell()->GetTriggeredByAuraSpellInfo();
        return aura && aura->Id == 954877;
    }

    void Targets(std::list<WorldObject*>& targets)
    {
        if (Collision())
        {
            targets.clear();
            if (Unit* target = GetExplTargetUnit())
                targets.push_back(target);
        }
    }

    void Knock(SpellEffIndex)
    {
        if (!Collision() && GetHitUnit() && GetHitUnit()->IsPlayer())
            GetCaster()->CastSpell(GetHitUnit(), 954877, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_area52_siegebreaker::Targets,
            EFFECT_0, TARGET_UNIT_CONE_ENEMY_104);
        OnEffectHitTarget += SpellEffectFn(spell_area52_siegebreaker::Knock, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

class aura_area52_siegebreaker_collision : public AuraScript
{
    PrepareAuraScript(aura_area52_siegebreaker_collision);
    bool Load() override
    {
        return GetCaster() && AscensionFreePick::Applies(GetCaster()->ToPlayer()) && GetUnitOwner()->IsPlayer();
    }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({954876}); }

    void Start(AuraEffect const*, AuraEffectHandleModes)
    {
        Previous.Relocate(GetTarget());
    }

    void Tick(AuraEffect const* effect)
    {
        PreventDefaultAction();
        Unit* caster = GetCaster();
        Unit* moving = GetTarget();
        if (!caster || !moving->IsAlive() || moving->GetExactDist(&Previous) < 0.01f)
            return;
        Previous.Relocate(moving);
        std::list<Unit*> candidates;
        float const range = moving->GetCombatReach() + 5.0f;
        Acore::AnyUnfriendlyUnitInObjectRangeCheck check(moving, caster, range);
        Acore::UnitListSearcher<Acore::AnyUnfriendlyUnitInObjectRangeCheck> searcher(moving, candidates, check);
        Cell::VisitObjects(moving, searcher, range);
        for (Unit* target : candidates)
        {
            if (target == moving || !target->IsAlive() || !caster->IsValidAttackTarget(target) ||
                !moving->IsWithinDistInMap(target, 0.0f) || !moving->IsWithinLOSInMap(target) ||
                !Hit.insert(target->GetGUID()).second)
                continue;
            caster->CastCustomSpell(954876, SPELLVALUE_BASE_POINT0, 200, target, true, nullptr, effect);
            if (!target->HasUnitState(UNIT_STATE_ROOT) && !target->GetVehicle() && !target->IsVehicle() &&
                (!target->IsCreature() || (!target->ToCreature()->IsImmuneToKnockback() &&
                !target->ToCreature()->isWorldBoss() && !target->ToCreature()->IsDungeonBoss())))
                target->KnockbackFrom(moving->GetPositionX(), moving->GetPositionY(),
                    GetSpellInfo()->Effects[EFFECT_0].MiscValue * 0.05f,
                    GetSpellInfo()->Effects[EFFECT_0].CalcValue(caster) * 0.05f);
        }
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_siegebreaker_collision::Start,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_siegebreaker_collision::Tick,
            EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }

    Position Previous;
    std::set<ObjectGuid> Hit;
};
