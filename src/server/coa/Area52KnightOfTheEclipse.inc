struct Area52EclipseMemory : DataMap::Base
{
    Area52KnightOfTheEclipse::Phase Last = Area52KnightOfTheEclipse::Phase::None;
};

constexpr char const* Area52EclipseMemoryKey = "Area52EclipseMemory";

bool HasArea52KnightOfTheEclipseHandlers(uint32 spell)
{
    using namespace Area52KnightOfTheEclipse;
    if (spell == KnightOfTheEclipse)
        return sSpellMgr->GetSpellProcEntry(SolarKnight) && sSpellMgr->GetSpellProcEntry(LunarKnight) &&
            sSpellMgr->GetSpellProcEntry(KnightHaste) && sSpellMgr->GetSpellInfo(SolarLock) &&
            sSpellMgr->GetSpellInfo(LunarLock) && sSpellMgr->GetSpellInfo(KnightfallStarfall) &&
            HasSpellScriptNamed(EclipseStrike, "spell_area52_eclipse_strike") &&
            HasSpellScriptNamed(SolarStrike, "spell_area52_solar_strike") &&
            HasSpellScriptNamed(KnightfallStar, "spell_dru_starfall_dummy") &&
            HasAuraScriptNamed(KnightHaste, "spell_area52_knight_haste") &&
            HasAuraScriptNamed(EclipseSolar, "spell_area52_eclipse_memory") &&
            HasAuraScriptNamed(EclipseLunar, "spell_area52_eclipse_memory");
    if (spell == 977831)
        return sSpellMgr->GetSpellInfo(SunfireMark) && HasAuraScriptNamed(Sunfire, "spell_area52_sunfire");
    if (spell == InfusedEnergy)
        return sSpellMgr->GetSpellProcEntry(InfusedEnergy) && sSpellMgr->GetSpellInfo(InfusedEnergyBuff) &&
            HasAuraScriptNamed(InfusedEnergy, "spell_area52_infused_energy");
    return false;
}

class spell_area52_eclipse_memory : public AuraScript
{
    PrepareAuraScript(spell_area52_eclipse_memory);

    void Remember(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !AscensionFreePick::Applies(player))
            return;
        player->CustomData.GetDefault<Area52EclipseMemory>(Area52EclipseMemoryKey)->Last =
            GetId() == Area52KnightOfTheEclipse::EclipseSolar ? Area52KnightOfTheEclipse::Phase::Solar :
            Area52KnightOfTheEclipse::Phase::Lunar;
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_eclipse_memory::Remember, EFFECT_0, SPELL_AURA_ANY,
            AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_eclipse_strike : public SpellScript
{
    PrepareSpellScript(spell_area52_eclipse_strike);

    bool Validate(SpellInfo const*) override
    {
        using namespace Area52KnightOfTheEclipse;
        return ValidateSpellInfo({KnightHaste, SolarKnight, LunarKnight, SolarLock, LunarLock, EclipseSolar,
            EclipseLunar});
    }

    void ScaleDamage()
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        Item const* weapon = player->GetWeaponForAttack(BASE_ATTACK, true);
        bool const twoHanded = weapon && weapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
        float const spellPower = float(player->SpellBaseDamageBonusDone(GetSpellInfo()->GetSchoolMask()));
        SetHitDamage(Area52KnightOfTheEclipse::EclipseStrikeDamage(GetHitDamage(), twoHanded, spellPower));
    }

    void Empower()
    {
        using namespace Area52KnightOfTheEclipse;
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        player->CastSpell(player, KnightHaste, true);
        if (player->HasAura(EclipseSolar))
        {
            player->CastSpell(player, SolarKnight, true);
            return;
        }
        if (player->HasAura(EclipseLunar))
        {
            player->CastSpell(player, LunarKnight, true);
            return;
        }
        auto* memory = player->CustomData.GetDefault<Area52EclipseMemory>(Area52EclipseMemoryKey);
        Phase const next = NextPhase(memory->Last);
        uint32 const lock = next == Phase::Solar ? SolarLock : LunarLock;
        if (player->HasAura(lock))
            return;
        player->CastSpell(player, next == Phase::Solar ? EclipseSolar : EclipseLunar, true);
        player->CastSpell(player, next == Phase::Solar ? SolarKnight : LunarKnight, true);
        player->CastSpell(player, lock, true);
        memory->Last = next;
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_eclipse_strike::ScaleDamage);
        AfterHit += SpellHitFn(spell_area52_eclipse_strike::Empower);
    }
};

class spell_area52_solar_strike : public SpellScript
{
    PrepareSpellScript(spell_area52_solar_strike);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52KnightOfTheEclipse::SolarBurn, Area52KnightOfTheEclipse::KnightfallStarfall});
    }

    void Burn()
    {
        using namespace Area52KnightOfTheEclipse;
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        Unit* target = GetHitUnit();
        if (!player || !target || !AscensionFreePick::Applies(player))
            return;
        uint8 const comboPoints = player->GetComboPoints();
        if (roll_chance_i(SolarBurnChance(comboPoints)))
            player->CastSpell(target, SolarBurn, true);
        if (!comboPoints || !player->HasAura(Knightfall))
            return;
        player->CastSpell(player, KnightfallStarfall, true);
        if (Aura* stars = player->GetAura(KnightfallStarfall))
        {
            stars->SetMaxDuration(KnightfallDurationMs(comboPoints));
            stars->SetDuration(KnightfallDurationMs(comboPoints));
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_solar_strike::Burn);
    }
};

class spell_area52_knight_haste : public AuraScript
{
    PrepareAuraScript(spell_area52_knight_haste);

    void SpendStack(AuraEffect const*, ProcEventInfo&)
    {
        PreventDefaultAction();
        ModStackAmount(-1);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_area52_knight_haste::SpendStack, EFFECT_0, SPELL_AURA_ADD_PCT_MODIFIER);
    }
};

class spell_area52_sunfire : public AuraScript
{
    PrepareAuraScript(spell_area52_sunfire);

    void Mark(AuraEffect const*, AuraEffectHandleModes)
    {
        if (Unit* caster = GetCaster())
            if (AscensionFreePick::Applies(caster->ToPlayer()))
                caster->CastSpell(GetTarget(), Area52KnightOfTheEclipse::SunfireMark, true);
    }

    void Unmark(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(Area52KnightOfTheEclipse::SunfireMark, GetCasterGUID());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_sunfire::Mark, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE,
            AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_sunfire::Unmark, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE,
            AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_infused_energy : public AuraScript
{
    PrepareAuraScript(spell_area52_infused_energy);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool NotEmpowered(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        SpellInfo const* buff = sSpellMgr->GetSpellInfo(Area52KnightOfTheEclipse::InfusedEnergyBuff);
        return spell && buff && !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) &&
            !spell->IsAffected(buff->SpellFamilyName, buff->Effects[EFFECT_1].SpellClassMask);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_infused_energy::NotEmpowered);
    }
};

class spell_area52_demonic_moonfire : public AuraScript
{
    PrepareAuraScript(spell_area52_demonic_moonfire);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        using namespace Area52KnightOfTheEclipse;
        return ValidateSpellInfo({DemonicMoonfireHelper, DemonicPower, WorldforgedCooldown});
    }

    bool Rolled(ProcEventInfo&)
    {
        using namespace Area52KnightOfTheEclipse;
        return !GetTarget()->HasAura(WorldforgedCooldown) &&
            roll_chance_i(sSpellMgr->GetSpellInfo(DemonicMoonfireHelper)->ProcChance);
    }

    void GrantPower(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        GetTarget()->CastCustomSpell(Area52KnightOfTheEclipse::DemonicPower, SPELLVALUE_BASE_POINT0,
            effect->GetAmount(), GetTarget(), true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_demonic_moonfire::Rolled);
        OnEffectProc += AuraEffectProcFn(spell_area52_demonic_moonfire::GrantPower, EFFECT_0,
            SPELL_AURA_ADD_FLAT_MODIFIER);
    }
};

class area52_improved_insect_swarm : public UnitScript
{
public:
    area52_improved_insect_swarm() : UnitScript("area52_improved_insect_swarm", true,
        {UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN, UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK}) { }

    static int32 BonusPct(Unit* target, Unit* attacker, SpellInfo const* info)
    {
        using namespace Area52KnightOfTheEclipse;
        if (!target || !attacker || !info || !AscensionFreePick::Applies(attacker->ToPlayer()))
            return 0;
        AuraEffect const* enchant = attacker->GetAuraEffect(ImprovedInsectSwarm, EFFECT_0);
        AuraEffect const* talent = attacker->GetAuraEffectOfRankedSpell(Area52DruidTalents::CelestialBane, EFFECT_0);
        int32 const amount = (enchant ? enchant->GetAmount() : 0) + (talent ? talent->GetAmount() : 0);
        if (!amount)
            return 0;
        int32 bonus = 0;
        if ((info->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE) && target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
            SPELLFAMILY_DRUID, InsectSwarmFlag0, 0, 0, attacker->GetGUID()))
            bonus += amount;
        if ((info->GetSchoolMask() & SPELL_SCHOOL_MASK_ARCANE) && target->GetAuraEffect(SPELL_AURA_PERIODIC_DAMAGE,
            SPELLFAMILY_DRUID, MoonfireFlag0, 0, 0, attacker->GetGUID()))
            bonus += amount;
        return bonus;
    }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* info) override
    {
        if (damage > 0)
            damage = Area52KnightOfTheEclipse::WithBonusPct(damage, BonusPct(target, attacker, info));
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* info) override
    {
        damage = uint32(Area52KnightOfTheEclipse::WithBonusPct(int32(damage), BonusPct(target, attacker, info)));
    }
};
