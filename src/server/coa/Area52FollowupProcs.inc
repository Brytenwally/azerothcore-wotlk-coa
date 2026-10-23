class spell_area52_followup_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_followup_proc);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({965489, 415011, 411119, 734493, 812316, 812317, 812318,
            812319, 986107, 863375, 863376, 863377, 20362, 811101, 920042, 920043,
            851562, 53390, 844405, 966120, 966121, 966122, 760192, 983791, 983792});
    }

    Area52FollowupProcs::Event Facts(ProcEventInfo& event)
    {
        Area52FollowupProcs::Event facts;
        Player* owner = GetTarget()->ToPlayer();
        facts.FreePick = owner && AscensionFreePick::Applies(owner);
        facts.OwnerIsActor = event.GetActor() == GetTarget();
        facts.OtherTarget = event.GetActionTarget() && event.GetActionTarget() != GetTarget();
        facts.FinalTidalWaves = GetTarget()->HasAura(51564);
        if (Aura* haste = GetTarget()->GetAura(966122))
            facts.NetherwindStacks = haste->GetStackAmount();
        facts.Periodic = (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0;
        facts.Damage = event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() > 0;
        facts.Healing = event.GetHealInfo() != nullptr;
        facts.Critical = (event.GetHitMask() & PROC_HIT_CRITICAL) != 0;
        facts.Root = event.GetSpellInfo() ? event.GetSpellInfo()->GetFirstRankSpell()->Id : 0;
        return facts;
    }

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52FollowupProcs::Trigger(GetId(), Facts(event)) != 0;
    }

    void HandleProc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (GetId() == 17958 && effect->GetEffIndex() != EFFECT_1)
            return;
        auto const facts = Facts(event);
        uint32 const trigger = Area52FollowupProcs::Trigger(GetId(), facts);
        Unit* owner = GetTarget();
        if (GetId() == 760191)
        {
            if (Unit* target = event.GetActionTarget())
                owner->CastSpell(target, 760192, true, nullptr, effect);
            return;
        }
        if (GetId() == 20042 || GetId() == 20045)
        {
            if (effect->GetEffIndex() != EFFECT_0 || !event.GetActionTarget())
                return;
            int32 amount = Area52FollowupProcs::TotemPower(owner->GetLevel(), GetId() == 20042 ? 1 : 2, true);
            owner->CastCustomSpell(owner, 920042, &amount, &amount, nullptr, true, nullptr, effect);
            owner->CastCustomSpell(event.GetActionTarget(), 920043, &amount, &amount, nullptr, true, nullptr, effect);
            return;
        }
        if (GetId() == 16219)
        {
            int32 duration = Area52FollowupProcs::TidalExtension(facts.Root);
            owner->CastCustomSpell(owner, trigger, &duration, nullptr, nullptr, true, nullptr, effect);
            return;
        }
        if (GetId() == 34493)
        {
            int32 reduction = -1000;
            owner->CastCustomSpell(owner, trigger, &reduction, &reduction, &reduction, true, nullptr, effect);
            return;
        }
        owner->CastSpell(owner, trigger, true, nullptr, effect);
        if (GetId() == 12318 || GetId() == 12857)
            if (Aura* buff = owner->GetAura(trigger, owner->GetGUID()))
            {
                buff->SetMaxDuration(10 * MINUTE * IN_MILLISECONDS);
                buff->SetDuration(buff->GetMaxDuration());
            }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_followup_proc::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_followup_proc::HandleProc, EFFECT_ALL, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_shared_might : public AuraScript
{
    PrepareAuraScript(spell_area52_shared_might);

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player) || GetAura()->IsSingleTarget())
            return;
        auto previous = player->GetSingleCastAuras();
        for (Aura* aura : previous)
            if (aura != GetAura() && aura->GetId() == 920043)
                aura->Remove();
        GetAura()->SetIsSingleTarget(true);
        player->GetSingleCastAuras().push_back(GetAura());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_shared_might::Apply, EFFECT_0,
            SPELL_AURA_MOD_ATTACK_POWER, AURA_EFFECT_HANDLE_REAL);
    }
};

class area52_followup_casts : public AllSpellScript
{
public:
    area52_followup_casts() : AllSpellScript("area52_followup_casts",
        {ALLSPELLHOOK_ON_SUCCESSFUL_INTERRUPT, ALLSPELLHOOK_ON_CAST, ALLSPELLHOOK_ON_CALCULATED_TARGET}) { }

    void OnSpellCalculatedTarget(Spell* spell, Unit* target, TargetInfo& hit) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player) || !target || hit.damage >= 0 ||
            hit.missCondition != SPELL_MISS_NONE)
            return;
        if (AuraEffect* dawn = target->GetAuraEffect(760192, EFFECT_0, player->GetGUID()))
            if (dawn->IsAffectedOnSpell(spell->GetSpellInfo()))
            {
                AddPct(hit.damage, dawn->GetAmount());
                dawn->GetBase()->Remove();
            }
    }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player) || !spell || spell->IsTriggered())
            return;
        uint32 rank = player->HasAura(52456) ? 3 : player->HasAura(16295) ? 2 : player->HasAura(16259) ? 1 : 0;
        if (!rank)
            return;
        bool summon = false;
        for (auto const& effect : info->Effects)
            if (effect.Effect == SPELL_EFFECT_SUMMON)
                if (auto const* properties = sSummonPropertiesStore.LookupEntry(effect.MiscValueB))
                    summon = summon || properties->Type == SUMMON_TYPE_TOTEM;
        if (!summon)
            return;
        for (auto const& guid : player->m_SummonSlot)
            if (Creature* totem = ObjectAccessor::GetCreature(*player, guid))
                if (totem->IsTotem() && totem->GetUInt32Value(UNIT_CREATED_BY_SPELL) == info->Id)
                {
                    int32 attack = Area52FollowupProcs::TotemPower(player->GetLevel(), rank, true);
                    int32 magic = Area52FollowupProcs::TotemPower(player->GetLevel(), rank, false);
                    player->CastCustomSpell(player, 816259, &attack, &magic, nullptr, true);
                    break;
                }
    }

    void OnSpellSuccessfulInterrupt(Spell* spell, Unit*) override
    {
        Player* player = spell && spell->GetCaster() ? spell->GetCaster()->ToPlayer() : nullptr;
        if (!player || !AscensionFreePick::Applies(player))
            return;
        if (spell->GetSpellInfo()->GetFirstRankSpell()->Id == 1766 && player->HasSpell(13867) &&
            player->HasAura(13867) && !spell->GetScriptValue(18425))
        {
            spell->SetScriptValue(18425, 1);
            player->CastSpell(player, 18425, true);
        }
        if (!Area52FollowupProcs::WindRush(AscensionFreePick::Applies(player),
            player->HasAura(32511), spell->GetSpellInfo()->GetFirstRankSpell()->Id, true) ||
            spell->GetScriptValue(411139))
            return;
        spell->SetScriptValue(411139, 1);
        player->CastSpell(player, 411139, true);
    }
};

class spell_area52_flame_throwing : public AuraScript
{
    PrepareAuraScript(spell_area52_flame_throwing);

    void Tick(AuraEffect const*)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !AscensionFreePick::Applies(player))
            return;
        PreventDefaultAction();
        if (Area52FollowupProcs::FlameReady(player->HasAura(812352), player->HasAura(811101), player->HasAura(811100)))
            player->CastSpell(player, 811100, true);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_area52_flame_throwing::Tick, EFFECT_2, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};
