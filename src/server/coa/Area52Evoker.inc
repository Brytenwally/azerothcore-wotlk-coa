struct Area52EvokerState : DataMap::Base
{
    Spell const* ShatterClaim = nullptr;
    bool ShatterWindowClaimed = false;
    Spell const* FrostyClaim = nullptr;
    Spell const* BrainFreezeClaim = nullptr;
};

constexpr char const* Area52EvokerStateKey = "Area52EvokerState";

bool HasArea52EvokerHandlers(uint32 spell)
{
    using namespace Area52Evoker;
    if (spell == Evoker)
        return sSpellMgr->GetSpellInfo(EvokerBuff) && sSpellMgr->GetSpellInfo(FrostEvoked) &&
            HasAuraScriptNamed(Evoker, "spell_area52_evoker") &&
            HasAuraScriptNamed(ArcaneEvoked, "spell_area52_arcane_evoked");
    if (spell == Shatter)
        return sSpellMgr->GetSpellInfo(ShatterBuff) != nullptr;
    if (spell == FrostyFrostFrostbolt)
        return sSpellMgr->GetSpellInfo(Chills) && sSpellMgr->GetSpellInfo(FrostyEmpowered) &&
            sSpellMgr->GetSpellInfo(FrostyDamage) && sSpellMgr->GetSpellInfo(FrostyProcChance);
    if (spell == ElementBoundMissiles)
        return sSpellMgr->GetSpellProcEntry(ElementBoundMissiles) &&
            HasAuraScriptNamed(ElementBoundMissiles, "spell_area52_element_bound_missiles");
    return spell == BrainFreezeEnchant || spell == CoolDown;
}

Area52Evoker::SpellKey Area52EvokerSpell(SpellInfo const* info)
{
    return {info->SpellFamilyName, info->SpellFamilyFlags[0], info->SpellFamilyFlags[1]};
}

Area52EvokerState* Area52EvokerStateOf(Player* player)
{
    return player->CustomData.GetDefault<Area52EvokerState>(Area52EvokerStateKey);
}

bool Area52SpentBuff(Spell const* spell, uint32 buff)
{
    for (Aura const* aura : spell->m_appliedMods)
        if (aura->GetId() == buff)
            return true;
    return false;
}

void Area52CastUnchargedBuff(Player* player, uint32 buff)
{
    player->CastSpell(player, buff, true);
    if (Aura* aura = player->GetAura(buff))
        aura->SetCharges(0);
}

int32 GetArea52CritDamageBonusPct(Unit const* caster, SpellInfo const* /*spell*/, Unit const* victim)
{
    Player* player = caster ? const_cast<Unit*>(caster)->ToPlayer() : nullptr;
    if (!player || !victim || !AscensionFreePick::Applies(player))
        return 0;
    Area52EvokerState const* state = player->CustomData.Get<Area52EvokerState>(Area52EvokerStateKey);
    if (!state || !state->ShatterClaim || state->ShatterClaim != player->m_spellModTakingSpell)
        return 0;
    SpellInfo const* buff = sSpellMgr->GetSpellInfo(Area52Evoker::ShatterBuff);
    if (!buff)
        return 0;
    return buff->Effects[victim->IsControlledByPlayer() ? EFFECT_2 : EFFECT_1].CalcValue();
}

class spell_area52_evoker : public AuraScript
{
    PrepareAuraScript(spell_area52_evoker);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool DrivenByCasts(ProcEventInfo&)
    {
        return false;
    }

    void ReachBrainFreeze(AuraEffect const*, SpellModifier*& mod)
    {
        if (mod)
            mod->mask = GetSpellInfo()->Effects[EFFECT_1].SpellClassMask |
                flag96(0, Area52Evoker::BrainFreezeTalentFlag1, 0);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_evoker::DrivenByCasts);
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_area52_evoker::ReachBrainFreeze, EFFECT_1,
            SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

class spell_area52_arcane_evoked : public AuraScript
{
    PrepareAuraScript(spell_area52_arcane_evoked);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void ReachBrainFreeze(AuraEffect const*, SpellModifier*& mod)
    {
        if (mod)
            mod->mask = flag96(0, Area52Evoker::BrainFreezeTalentFlag1, 0);
    }

    void Register() override
    {
        DoEffectCalcSpellMod += AuraEffectCalcSpellModFn(spell_area52_arcane_evoked::ReachBrainFreeze, EFFECT_1,
            SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

class spell_area52_element_bound_missiles : public AuraScript
{
    PrepareAuraScript(spell_area52_element_bound_missiles);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        using namespace Area52Evoker;
        return ValidateSpellInfo({ElementBoundMissilesHelper, ElementBoundHaste, ElementBoundMana,
            WorldforgedCooldown});
    }

    bool Rolled(ProcEventInfo&)
    {
        using namespace Area52Evoker;
        return !GetTarget()->HasAura(WorldforgedCooldown) &&
            roll_chance_i(sSpellMgr->GetSpellInfo(ElementBoundMissilesHelper)->ProcChance);
    }

    void GrantPower(AuraEffect const* effect, ProcEventInfo&)
    {
        using namespace Area52Evoker;
        PreventDefaultAction();
        Unit* mage = GetTarget();
        mage->CastCustomSpell(ElementBoundHaste, SPELLVALUE_BASE_POINT0, effect->GetAmount(), mage, true, nullptr,
            effect);
        if (AuraEffect const* mana = GetAura()->GetEffect(EFFECT_1))
            mage->CastCustomSpell(ElementBoundMana, SPELLVALUE_BASE_POINT0, mana->GetAmount(), mage, true, nullptr,
                effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_element_bound_missiles::Rolled);
        OnEffectProc += AuraEffectProcFn(spell_area52_element_bound_missiles::GrantPower, EFFECT_0,
            SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

class area52_evoker_damage : public UnitScript
{
public:
    area52_evoker_damage() : UnitScript("area52_evoker_damage", true, {UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN}) { }

    static int32 BonusPct(Unit* target, Player* mage, SpellInfo const* info)
    {
        using namespace Area52Evoker;
        int32 bonus = 0;
        Area52EvokerState const* state = mage->CustomData.Get<Area52EvokerState>(Area52EvokerStateKey);
        if (state && state->BrainFreezeClaim && state->BrainFreezeClaim == mage->m_spellModTakingSpell)
            if (AuraEffect const* enchant = mage->GetAuraEffect(BrainFreezeEnchant, EFFECT_1))
                bonus += enchant->GetAmount();
        if (IsFrostbolt(Area52EvokerSpell(info)))
            if (AuraEffect const* coolDown = mage->GetAuraEffect(CoolDown, EFFECT_0))
                if (target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE, SPELLFAMILY_MAGE, MageFireballFlag0, 0, 0,
                    mage->GetGUID()))
                    bonus += coolDown->GetAmount();
        return bonus;
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* info) override
    {
        Player* mage = attacker ? attacker->ToPlayer() : nullptr;
        if (damage <= 0 || !target || !info || !mage || !AscensionFreePick::Applies(mage))
            return;
        if (int32 const bonus = BonusPct(target, mage, info))
            AddPct(damage, bonus);
    }
};

class area52_evoker_casts : public AllSpellScript
{
public:
    area52_evoker_casts() : AllSpellScript("area52_evoker_casts",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CAST, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    static Player* Mage(Spell const* spell)
    {
        Player* player = spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        return player && AscensionFreePick::Applies(player) ? player : nullptr;
    }

    void OnSpellBeforeEffects(Spell* spell, Unit*, SpellInfo const* info) override
    {
        using namespace Area52Evoker;
        Player* mage = Mage(spell);
        if (!mage)
            return;
        Area52EvokerState* state = Area52EvokerStateOf(mage);
        if (state->ShatterClaim == spell)
            state->ShatterClaim = nullptr;
        if (state->FrostyClaim == spell)
            state->FrostyClaim = nullptr;
        if (state->BrainFreezeClaim == spell)
            state->BrainFreezeClaim = nullptr;
        if (spell->IsTriggered())
            return;

        SpellKey const key = Area52EvokerSpell(info);
        bool const spentBarrage = IsArcaneMissiles(key) && Area52SpentBuff(spell, MissileBarrage);
        bool const spentFreeze = (IsFireball(key) || IsFrostfireBolt(key)) && Area52SpentBuff(spell, BrainFreeze);
        if (mage->HasAura(Evoker))
        {
            if (spentBarrage)
                Area52CastUnchargedBuff(mage, ArcaneEvoked);
            if (spentFreeze && IsFireball(key))
                Area52CastUnchargedBuff(mage, FireEvoked);
            if (spentBarrage || spentFreeze)
                mage->CastSpell(mage, FrostEvoked, true);
        }
        if (spentFreeze && IsFireball(key) && mage->HasAura(BrainFreezeEnchant))
            state->BrainFreezeClaim = spell;
        if (mage->HasAura(ShatterBuff) && ClaimsShatter(key, false, state->ShatterWindowClaimed))
        {
            state->ShatterClaim = spell;
            state->ShatterWindowClaimed = true;
        }
    }

    static void EmpowerFrostbolt(Player* mage, Spell* spell, Area52EvokerState* state)
    {
        using namespace Area52Evoker;
        if (mage->HasAura(FrostyEmpowered))
        {
            mage->RemoveAurasDueToSpell(FrostyEmpowered);
            state->FrostyClaim = spell;
            mage->CastSpell(mage, FrostyProcChance, true);
            if (Aura* chance = mage->GetAura(FrostyProcChance))
            {
                int32 const duration = FrostyChanceDurationMs(spell->GetDelayMoment());
                chance->SetMaxDuration(duration);
                chance->SetDuration(duration);
            }
        }
        Aura const* chills = mage->GetAura(Chills);
        ChillStep const step = AddChill(chills ? chills->GetStackAmount() : 0);
        if (step.Empowers)
        {
            mage->RemoveAurasDueToSpell(Chills);
            mage->CastSpell(mage, FrostyEmpowered, true);
            return;
        }
        Area52CastUnchargedBuff(mage, Chills);
    }

    void OnSpellCast(Spell* spell, Unit*, SpellInfo const* info, bool) override
    {
        using namespace Area52Evoker;
        Player* mage = Mage(spell);
        if (!mage || spell->IsTriggered())
            return;
        Area52EvokerState* state = Area52EvokerStateOf(mage);
        if (state->ShatterClaim == spell)
            mage->RemoveAurasDueToSpell(ShatterBuff);
        if (!IsFrostbolt(Area52EvokerSpell(info)))
            return;
        if (mage->HasAura(Evoker) && mage->HasAura(FrostEvoked))
        {
            mage->RemoveAurasDueToSpell(FrostEvoked);
            Area52CastUnchargedBuff(mage, EvokerBuff);
        }
        if (mage->HasAura(Shatter))
        {
            mage->CastSpell(mage, ShatterBuff, true);
            state->ShatterWindowClaimed = false;
        }
        if (mage->HasAura(FrostyFrostFrostbolt))
            EmpowerFrostbolt(mage, spell, state);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 missInfo, uint32, uint32, bool) override
    {
        using namespace Area52Evoker;
        Player* mage = Mage(spell);
        if (!mage || spell->IsTriggered() || !IsFrostbolt(Area52EvokerSpell(spell->GetSpellInfo())))
            return;
        Area52EvokerState* state = Area52EvokerStateOf(mage);
        if (!state->ShatterWindowClaimed)
            mage->RemoveAurasDueToSpell(ShatterBuff);
        if (state->FrostyClaim != spell)
            return;
        state->FrostyClaim = nullptr;
        if (target && missInfo == SPELL_MISS_NONE)
            mage->CastSpell(target, FrostyDamage, true);
        mage->RemoveAurasDueToSpell(FrostyProcChance);
    }
};

class area52_evoked_triggers : public UnitScript
{
public:
    area52_evoked_triggers() : UnitScript("area52_evoked_triggers", true, {UNITHOOK_ON_AURA_APPLY}) { }

    void OnAuraApply(Unit* unit, Aura* aura) override
    {
        using namespace Area52Evoker;
        if (!unit || !aura || !AscensionFreePick::Applies(unit->ToPlayer()))
            return;
        if (aura->GetId() == BrainFreeze)
            unit->RemoveAurasDueToSpell(ArcaneEvoked);
        else if (aura->GetId() == MissileBarrage)
            unit->RemoveAurasDueToSpell(FireEvoked);
    }
};
