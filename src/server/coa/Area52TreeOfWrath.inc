struct Area52LightningRods : DataMap::Base
{
    std::vector<ObjectGuid> Targets;
};

constexpr char const* Area52LightningRodsKey = "Area52LightningRods";

bool HasArea52TreeOfWrathHandlers(uint32 spell)
{
    using namespace Area52TreeOfWrath;
    if (spell == TreeOfWrath)
        return sSpellMgr->GetSpellProcEntry(TreeOfWrathEmpowerment) && sSpellMgr->GetSpellInfo(LightningRod) &&
            sSpellMgr->GetSpellInfo(InstantLightningBolt) && sSpellMgr->GetSpellInfo(TreeOfWrathHitChance) &&
            HasAuraScriptNamed(TreeOfWrathForm, "spell_area52_tree_of_wrath_form") &&
            HasAuraScriptNamed(TreeOfWrathEmpowerment, "spell_area52_tree_of_wrath") &&
            HasSpellScriptNamed(LivingLightningDamage.back(), "spell_area52_living_lightning_rod");
    if (spell == NaturesFlow)
        return sSpellMgr->GetSpellProcEntry(NaturesFlow) && sSpellMgr->GetSpellInfo(NaturesFlowSwiftness) &&
            sSpellMgr->GetSpellInfo(NaturesFlowCritical);
    if (spell == ChargedTides)
        return sSpellMgr->GetSpellInfo(ChargedTidesStack) != nullptr;
    if (spell == BlessingOfTheEternals)
        return HasAuraScriptNamed(51940, "spell_sha_earthliving_weapon");
    if (spell == TranscendentalEmbrace)
        return HasAuraScriptNamed(TranscendentalEmbrace, "spell_area52_transcendental_embrace") &&
            HasAuraScriptNamed(TranscendentalEmbraceBuff, "spell_area52_transcendental_jump");
    return false;
}

bool IsArea52ShamanSpell(SpellInfo const* spell, uint32 flags0)
{
    return spell && spell->SpellFamilyName == SPELLFAMILY_SHAMAN && (spell->SpellFamilyFlags[0] & flags0);
}

uint32 Area52HighestKnownRank(Player const* player, uint32 firstRank)
{
    uint32 highest = 0;
    for (uint32 rank = firstRank; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
        if (player->HasSpell(rank))
            highest = rank;
    return highest;
}

void Area52CutLivingLightningCooldown(Player* player)
{
    using namespace Area52TreeOfWrath;
    for (uint32 rank = LivingLightning; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
    {
        uint32 const remaining = player->GetSpellCooldownDelay(rank);
        if (!remaining)
            continue;
        if (CooldownAfterCut(remaining, LivingLightningCooldownCutMs))
            player->ModifySpellCooldown(rank, -LivingLightningCooldownCutMs);
        else
            player->RemoveSpellCooldown(rank, true);
    }
}

class spell_area52_tree_of_wrath_form : public AuraScript
{
    PrepareAuraScript(spell_area52_tree_of_wrath_form);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52TreeOfWrath::InstantLightningBolt, Area52TreeOfWrath::TreeOfWrathHitChance});
    }

    void Empower(AuraEffect const* effect, AuraEffectHandleModes)
    {
        using namespace Area52TreeOfWrath;
        Unit* target = GetTarget();
        target->CastSpell(target, InstantLightningBolt, true, nullptr, effect);
        target->CastCustomSpell(TreeOfWrathHitChance, SPELLVALUE_BASE_POINT0, TreeOfWrathHitChancePct, target, true,
            nullptr, effect);
    }

    void Unempower(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(Area52TreeOfWrath::InstantLightningBolt);
        GetTarget()->RemoveAurasDueToSpell(Area52TreeOfWrath::TreeOfWrathHitChance);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_tree_of_wrath_form::Empower, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_tree_of_wrath_form::Unempower, EFFECT_0,
            SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_tree_of_wrath : public AuraScript
{
    PrepareAuraScript(spell_area52_tree_of_wrath);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52TreeOfWrath::LightningRod, Area52TreeOfWrath::LivingLightning});
    }

    static void RememberRod(Player* player, Unit* target)
    {
        auto& rods = player->CustomData.GetDefault<Area52LightningRods>(Area52LightningRodsKey)->Targets;
        if (std::find(rods.begin(), rods.end(), target->GetGUID()) == rods.end())
            rods.push_back(target->GetGUID());
    }

    static void StrikeRods(Player* player, AuraEffect const* effect)
    {
        using namespace Area52TreeOfWrath;
        auto& rods = player->CustomData.GetDefault<Area52LightningRods>(Area52LightningRodsKey)->Targets;
        std::vector<Unit*> rodded;
        rods.erase(std::remove_if(rods.begin(), rods.end(), [&](ObjectGuid const& guid)
        {
            Unit* target = ObjectAccessor::GetUnit(*player, guid);
            if (!target || !target->IsAlive() || !target->HasAura(LightningRod, player->GetGUID()))
                return true;
            rodded.push_back(target);
            return false;
        }), rods.end());
        uint32 const bolt = Area52HighestKnownRank(player, LightningBolt);
        if (!bolt)
            return;
        for (Unit* target : rodded)
            player->CastSpell(target, bolt, true, nullptr, effect);
    }

    void HandleProc(AuraEffect const* effect, ProcEventInfo& event)
    {
        using namespace Area52TreeOfWrath;
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        if (!player || !spell)
            return;
        if (DamageInfo* damage = event.GetDamageInfo())
        {
            Unit* target = event.GetActionTarget();
            if (damage->GetDamage() && target && target != player &&
                IsArea52ShamanSpell(spell, ShamanLightningBoltFlag0))
            {
                player->CastSpell(target, LightningRod, true, nullptr, effect);
                RememberRod(player, target);
            }
            return;
        }
        if (!event.GetHealInfo() || !(event.GetHitMask() & PROC_HIT_CRITICAL) ||
            (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC))
            return;
        if (spell->GetSchoolMask() & SPELL_SCHOOL_MASK_NATURE)
            StrikeRods(player, effect);
        if (IsArea52ShamanSpell(spell, LivingLightningCooldownCutFlags0))
            Area52CutLivingLightningCooldown(player);
    }

    void Register() override
    {
        OnEffectProc += AuraEffectProcFn(spell_area52_tree_of_wrath::HandleProc, EFFECT_2,
            SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_living_lightning_rod : public SpellScript
{
    PrepareSpellScript(spell_area52_living_lightning_rod);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetCaster() ? GetCaster()->ToPlayer() : nullptr);
    }

    void StrikeRod()
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        int32 const damage = GetHitDamage();
        if (!caster || !target || damage <= 0)
            return;
        AuraEffect const* rod = target->GetAuraEffect(Area52TreeOfWrath::LightningRod, EFFECT_0, caster->GetGUID());
        if (!rod || rod->GetAmount() <= 0)
            return;
        SpellInfo const* info = GetSpellInfo();
        SpellSchoolMask const school = info->GetSchoolMask();
        bool const crit = roll_chance_f(target->SpellTakenCritChance(caster, info, school,
            caster->SpellDoneCritChance(target, info, school, BASE_ATTACK, false), BASE_ATTACK, false));
        SpellNonMeleeDamage log(caster, target, info, school);
        caster->CalculateSpellDamageTaken(&log, CalculatePct(damage, rod->GetAmount()), info, BASE_ATTACK, crit);
        Unit::DealDamageMods(target, log.damage, &log.absorb);
        caster->SendSpellNonMeleeDamageLog(&log);
        caster->DealSpellDamage(&log, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_area52_living_lightning_rod::StrikeRod);
    }
};

class spell_area52_transcendental_embrace : public AuraScript
{
    PrepareAuraScript(spell_area52_transcendental_embrace);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool CriticalHeal(ProcEventInfo& event)
    {
        using namespace Area52TreeOfWrath;
        SpellInfo const* spell = event.GetSpellInfo();
        if (!spell || !event.GetHealInfo() || !(event.GetHitMask() & PROC_HIT_CRITICAL))
            return false;
        if (spell->SpellFamilyName == SPELLFAMILY_DRUID)
            return spell->SpellFamilyFlags[0] & TranscendentalDruidFlags0;
        return IsArea52ShamanSpell(spell, TranscendentalShamanFlags0);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_transcendental_embrace::CriticalHeal);
    }
};

class spell_area52_transcendental_jump : public AuraScript
{
    PrepareAuraScript(spell_area52_transcendental_jump);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    void AddJump(AuraEffect const*, AuraEffectHandleModes)
    {
        using namespace Area52TreeOfWrath;
        Player* player = GetTarget()->ToPlayer();
        if (!player || _jump)
            return;
        _jump = std::make_unique<SpellModifier>(GetAura());
        _jump->op = SPELLMOD_JUMP_TARGETS;
        _jump->type = SPELLMOD_FLAT;
        _jump->value = TranscendentalExtraJumps;
        _jump->spellId = GetId();
        _jump->mask = flag96(ShamanChainHealFlag0, 0, 0);
        player->AddSpellMod(_jump.get(), true);
    }

    void RemoveJump(AuraEffect const*, AuraEffectHandleModes)
    {
        if (!_jump)
            return;
        if (Player* player = GetTarget()->ToPlayer())
            player->AddSpellMod(_jump.get(), false);
        _jump.reset();
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_transcendental_jump::AddJump, EFFECT_0,
            SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_transcendental_jump::RemoveJump, EFFECT_0,
            SPELL_AURA_ADD_FLAT_MODIFIER, AURA_EFFECT_HANDLE_REAL);
    }

    std::unique_ptr<SpellModifier> _jump;
};

class area52_tree_of_wrath_casts : public AllSpellScript
{
public:
    area52_tree_of_wrath_casts() : AllSpellScript("area52_tree_of_wrath_casts",
        {ALLSPELLHOOK_ON_BEFORE_EFFECTS, ALLSPELLHOOK_ON_CAST}) { }

    void OnSpellBeforeEffects(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        using namespace Area52TreeOfWrath;
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || spell->IsTriggered() || !AscensionFreePick::Applies(player))
            return;
        AuraEffect const* flow = player->GetAuraEffect(NaturesFlow, EFFECT_0);
        SpellInfo const* swiftness = sSpellMgr->GetSpellInfo(NaturesFlowSwiftness);
        if (!flow || !swiftness || !info->IsAffected(swiftness->SpellFamilyName,
            swiftness->Effects[EFFECT_0].SpellClassMask))
            return;
        for (Aura const* aura : spell->m_appliedMods)
            if (aura->GetId() == NaturesSwiftness)
            {
                player->CastSpell(player, NaturesFlowCritical, true, nullptr, flow);
                return;
            }
    }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        using namespace Area52TreeOfWrath;
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        bool const triggered = spell->IsTriggered();
        if (!triggered && player->GetShapeshiftForm() == FORM_TREE_OF_WRATH && player->HasAura(InstantLightningBolt) &&
            IsArea52ShamanSpell(info, ShamanLightningBoltFlag0))
            for (uint32 rank = LightningBolt; rank; rank = sSpellMgr->GetNextSpellInChain(rank))
                if (player->HasSpell(rank))
                    player->AddSpellCooldown(rank, 0, LightningBoltCooldownMs, true);
        if (player->HasAura(ChargedTides))
        {
            bool const overload = IsLightningOverload(info->Id);
            if (overload || IsArea52ShamanSpell(info, TreeOfWrathLightningFlags0))
                for (uint8 stacks = ChargedTidesStacksFor(triggered, overload); stacks; --stacks)
                    player->CastSpell(player, ChargedTidesStack, true);
        }
        if (!triggered && IsArea52ShamanSpell(info, ChargedTidesSpenderFlags0))
            player->RemoveAurasDueToSpell(ChargedTidesStack);
    }
};
