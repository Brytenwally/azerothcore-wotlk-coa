bool HasArea52LegendaryBatchHandlers(uint32 spell)
{
    std::vector<std::pair<uint32, std::string>> required;
    std::vector<std::pair<uint32, std::string>> spellScripts;
    std::vector<uint32> helpers;
    switch (spell)
    {
        case 84510:
            required.emplace_back(84748, "spell_area52_slinger_consumption");
            helpers = {84748, 84511};
            break;
        case 965503:
            required.emplace_back(983908, "spell_area52_slinger_consumption");
            helpers = {983908, 965509};
            break;
        case 965504:
            required.emplace_back(965706, "spell_area52_bone_chill_vulnerability");
            helpers = {965510, 965706, 74396};
            break;
        case 965505:
            helpers = {983895, 983896};
            break;
        case 81119:
            helpers = {81120, 93582, 81121};
            break;
        case 81122:
            helpers = {81123, 81124, 954046, 954048};
            required.emplace_back(954046, "spell_area52_overloaded_mind");
            spellScripts.emplace_back(954048, "spell_area52_froststorm_echo");
            break;
        case 81479:
            helpers = {982220, 81125, 954047, 954049};
            break;
        case 965315:
            helpers = {965316, 954434, 984996};
            required.emplace_back(965315, "spell_area52_felfire_equipment");
            for (uint32 rank : {944811u, 944812u, 944813u, 944814u})
            {
                helpers.push_back(rank);
                spellScripts.emplace_back(rank, "spell_area52_pure_chaos_bolt");
            }
            spellScripts.emplace_back(411119, "spell_area52_felfire_emberstorm");
            break;
        case 99634:
            helpers = {99636, 99638, 954340, 300600, 986146};
            required.emplace_back(5487, "spell_area52_corrupted_bear_form");
            required.emplace_back(9634, "spell_area52_corrupted_bear_form");
            required.emplace_back(17941, "spell_area52_corrupted_bear_instant");
            required.emplace_back(34936, "spell_area52_corrupted_bear_instant");
            break;
        default:
            return false;
    }
    required.emplace_back(spell, spell == 99634 ? "spell_area52_corrupted_bear" : "spell_area52_legendary_batch");
    if (!sSpellMgr->GetSpellProcEntry(spell))
        return false;
    for (uint32 helper : helpers)
        if (!sSpellMgr->GetSpellInfo(helper))
            return false;
    for (auto const& [id, name] : required)
    {
        std::list<AuraScript*> scripts;
        sScriptMgr->CreateAuraScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == name;
            delete script;
        }
        if (!found)
            return false;
    }
    for (auto const& [id, name] : spellScripts)
    {
        std::list<SpellScript*> scripts;
        sScriptMgr->CreateSpellScripts(id, scripts);
        bool found = false;
        for (auto* script : scripts)
        {
            found |= *script->_GetScriptName() == name;
            delete script;
        }
        if (!found)
            return false;
    }
    return true;
}

class spell_area52_legendary_batch : public AuraScript
{
    PrepareAuraScript(spell_area52_legendary_batch);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({84748, 983908, 965510, 74396, 983895, 983896, 81120, 93582, 81121,
            81123, 982220, 81124, 81125, 965316});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* owner = GetTarget()->ToPlayer();
        Unit* target = event.GetActionTarget();
        SpellInfo const* spell = event.GetSpellInfo();
        if (GetId() == 965315 && owner && spell && spell->Id == 965316)
            return event.GetActor() == owner && target && owner->IsValidAttackTarget(target) &&
                event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
        if (!owner || !spell || !Area52LegendaryBatch::Direct(event.GetActor() == owner,
            target && owner->IsValidAttackTarget(target),
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage(),
            (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0,
            event.GetProcSpell() && event.GetProcSpell()->IsTriggered()))
            return false;
        if (GetId() == 965505)
            return !target->HasAura(983896, owner->GetGUID());
        if (GetId() == 965315)
            return spell->GetFirstRankSpell()->Id == 50796 || spell->GetFirstRankSpell()->Id == 944811 ||
                spell->Id == 300601;
        if (GetId() == 81122 || GetId() == 81479)
        {
            uint32 const schools = SPELL_SCHOOL_MASK_NATURE |
                (GetId() == 81122 ? SPELL_SCHOOL_MASK_FROST : SPELL_SCHOOL_MASK_ARCANE);
            return (spell->SchoolMask & schools) && event.GetProcSpell() &&
                (GetId() != 81479 || !event.GetProcSpell()->GetScriptValue(81479));
        }
        if (GetId() == 81119)
        {
            Aura* stacks = target->GetAura(93582, owner->GetGUID());
            return Area52LegendaryBatch::Overheating(spell->DmgClass == SPELL_DAMAGE_CLASS_MELEE,
                spell->SpellFamilyName == SPELLFAMILY_MAGE && (spell->SchoolMask & SPELL_SCHOOL_MASK_FIRE),
                stacks ? stacks->GetStackAmount() : 0, sSpellMgr->GetSpellInfo(93582)->StackAmount);
        }
        return true;
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* owner = GetTarget()->ToPlayer();
        Unit* target = event.GetActionTarget();
        uint8 const dispatch = GetId() == 965503 ? EFFECT_2 :
            (GetId() == 84510 || GetId() == 965315) ? EFFECT_1 : EFFECT_0;
        if (!owner || !target || effect->GetEffIndex() != dispatch)
            return;
        if (uint32 clearcast = Area52LegendaryBatch::Clearcast(GetId()))
        {
            owner->CastSpell(owner, clearcast, true, nullptr, effect);
            if (Aura* aura = owner->GetAura(clearcast, owner->GetGUID()))
                aura->SetCharges(GetId() == 965503 ? 2 : 1);
            return;
        }
        if (GetId() == 965504)
        {
            owner->CastSpell(target, 965510, true, nullptr, effect);
            if (owner->HasSpell(44543) || owner->HasSpell(44545))
                owner->CastSpell(owner, 74396, true, nullptr, effect);
            return;
        }
        if (GetId() == 81122 || GetId() == 81479)
        {
            if (GetId() == 81479)
                const_cast<Spell*>(event.GetProcSpell())->SetScriptValue(81479, 1);
            uint32 const stack = GetId() == 81122 ? 81123 : 982220;
            owner->CastSpell(owner, stack, true, nullptr, effect);
            if (Aura* aura = owner->GetAura(stack, owner->GetGUID()))
                if (aura->GetStackAmount() >= Area52LegendaryBatch::Threshold(!owner->GetCombatManager().GetPvPCombatRefs().empty()))
                {
                    aura->Remove();
                    owner->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                        GetId() == 81122 ? 81124 : 81125, true, nullptr, effect);
                }
            return;
        }
        if (GetId() == 965315)
        {
            if (event.GetSpellInfo()->Id == 965316)
            {
                owner->CastSpell(owner, 954434, true, nullptr, effect);
                return;
            }
            owner->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                965316, true, nullptr, effect);
            return;
        }
        if (GetId() == 965505)
        {
            owner->CastSpell(target, 983896, true, nullptr, effect);
            owner->CastSpell(target, 983895, true, nullptr, effect);
            return;
        }
        if (GetId() == 81119)
        {
            SpellInfo const* spell = event.GetSpellInfo();
            if (spell->DmgClass == SPELL_DAMAGE_CLASS_MELEE)
            {
                owner->CastSpell(target, 81120, true, nullptr, effect);
                owner->AddAura(93582, target);
            }
            else
                owner->CastSpell(target, 81121, true, nullptr, effect);
        }
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (uint32 clearcast = Area52LegendaryBatch::Clearcast(GetId()))
            GetTarget()->RemoveAurasDueToSpell(clearcast, GetTarget()->GetGUID());
        for (uint32 helper : GetId() == 81122 ? std::vector<uint32>{81123, 954046} :
            GetId() == 81479 ? std::vector<uint32>{982220, 954047} :
            GetId() == 965315 ? std::vector<uint32>{954434} : std::vector<uint32>{})
            GetTarget()->RemoveAurasDueToSpell(helper, GetTarget()->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_legendary_batch::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_legendary_batch::Proc,
            EFFECT_ALL, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_legendary_batch::Remove,
            EFFECT_ALL, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_slinger_consumption : public AuraScript
{
    PrepareAuraScript(spell_area52_slinger_consumption);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool CheckProc(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        Spell const* cast = event.GetProcSpell();
        return event.GetActor() == GetTarget() && spell && cast && !cast->IsTriggered() &&
            (spell->PowerType == POWER_MANA || (GetId() == 983908 &&
                (spell->PowerType == POWER_ENERGY || spell->PowerType == POWER_RAGE))) &&
            (spell->ManaCost || spell->ManaCostPercentage) &&
            (spell->SchoolMask & GetSpellInfo()->Effects[EFFECT_0].MiscValue);
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (effect->GetEffIndex() != EFFECT_2)
            return;
        Unit* owner = GetTarget();
        Unit* target = event.GetActionTarget();
        if (target && owner->IsValidAttackTarget(target))
        {
            if (GetId() == 84748)
                owner->CastSpell(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(),
                    84511, true, nullptr, effect);
            else
                owner->CastSpell(target, 965509, true, nullptr, effect);
        }
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_slinger_consumption::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_slinger_consumption::Proc,
            EFFECT_2, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_bone_chill_vulnerability : public AuraScript
{
    PrepareAuraScript(spell_area52_bone_chill_vulnerability);

    void Amount(AuraEffect const* effect, int32& amount, bool&)
    {
        Player* caster = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!caster || !AscensionFreePick::Applies(caster))
            return;
        bool const pvp = GetUnitOwner() && GetUnitOwner()->IsControlledByPlayer();
        if ((pvp && effect->GetEffIndex() == EFFECT_0) || (!pvp && effect->GetEffIndex() == EFFECT_2))
            amount = 0;
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_area52_bone_chill_vulnerability::Amount,
            EFFECT_ALL, SPELL_AURA_MOD_DAMAGE_FROM_CASTER);
    }
};

class area52_legendary_batch_metadata : public GlobalScript
{
public:
    area52_legendary_batch_metadata() : GlobalScript("area52_legendary_batch_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        if (info->Id == 983908)
            info->Effects[EFFECT_0].MiscValue = SPELL_SCHOOL_MASK_ALL;
        if (info->Id == 81123 || info->Id == 982220)
            info->StackAmount = 10;
        if (info->Id == 81479)
            info->Effects[EFFECT_0].TriggerSpell = 982220;
        if (info->Id == 965509)
            info->SpellFamilyFlags[1] |= 32768;
        if (info->Id == 81125)
        {
            info->Effects[EFFECT_0].RadiusEntry = sSpellRadiusStore.LookupEntry(14);
            info->Effects[EFFECT_2].RadiusEntry = sSpellRadiusStore.LookupEntry(14);
        }
        if (info->Id == 954048)
        {
            info->Effects[EFFECT_0].BasePoints = -1;
            info->Effects[EFFECT_0].RealPointsPerLevel = 3.0f;
        }
        if (info->Id == 300600)
        {
            info->SpellFamilyName = SPELLFAMILY_WARLOCK;
            info->Effects[EFFECT_0].ApplyAuraName = SPELL_AURA_MOD_IGNORE_SHAPESHIFT;
            info->Effects[EFFECT_0].SpellClassMask = flag96(1, 0, 0);
        }
        if (info->Id == 99634)
            info->Effects[EFFECT_1].Effect = 0;
        if (info->Id == 954340)
            info->Effects[EFFECT_2].Effect = 0;
    }
};

class spell_area52_overloaded_mind : public AuraScript
{
    PrepareAuraScript(spell_area52_overloaded_mind);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool CheckProc(ProcEventInfo& event)
    {
        SpellInfo const* spell = event.GetSpellInfo();
        return GetTarget()->HasAura(81122) && spell &&
            (spell->SchoolMask & (SPELL_SCHOOL_MASK_NATURE | SPELL_SCHOOL_MASK_FROST)) &&
            Area52LegendaryBatch::Direct(event.GetActor() == GetTarget(),
                event.GetActionTarget() && GetTarget()->IsValidAttackTarget(event.GetActionTarget()),
                event.GetDamageInfo() && event.GetDamageInfo()->GetDamage(),
                (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0,
                event.GetProcSpell() && event.GetProcSpell()->IsTriggered());
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        int32 amount = int32(std::min<uint64>(INT32_MAX,
            uint64(event.GetDamageInfo()->GetDamage()) * uint32(std::max(0, effect->GetAmount())) / 100));
        if (amount)
            GetTarget()->CastCustomSpell(event.GetActionTarget(), 954048, &amount, nullptr, nullptr,
                true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_overloaded_mind::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_overloaded_mind::Proc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_area52_froststorm_echo : public SpellScript
{
    PrepareSpellScript(spell_area52_froststorm_echo);

    void Hit()
    {
        SpellInfo const* source = GetSpell()->GetTriggeredByAuraSpellInfo();
        if (AscensionFreePick::Applies(GetCaster()->ToPlayer()) && source && source->Id == 954046)
            SetHitDamage(GetSpellValue()->EffectBasePoints[EFFECT_0] + 1);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_froststorm_echo::Hit);
    }
};

namespace Area52CorruptedBear
{
bool Active(Unit* owner)
{
    return AscensionFreePick::Applies(owner ? owner->ToPlayer() : nullptr) && owner->HasAura(99634) &&
        (owner->GetShapeshiftForm() == FORM_BEAR || owner->GetShapeshiftForm() == FORM_DIREBEAR);
}

void Synchronize(Unit* owner)
{
    if (!AscensionFreePick::Applies(owner ? owner->ToPlayer() : nullptr))
        return;
    if (Active(owner))
    {
        if (!owner->HasAura(954340, owner->GetGUID()))
            owner->CastSpell(owner, 954340, true);
        if (owner->HasAura(17941) || owner->HasAura(34936))
        {
            if (!owner->HasAura(300600, owner->GetGUID()))
                owner->CastSpell(owner, 300600, true);
        }
        else
            owner->RemoveAurasDueToSpell(300600, owner->GetGUID());
    }
    else
    {
        owner->RemoveAurasDueToSpell(954340, owner->GetGUID());
        owner->RemoveAurasDueToSpell(300600, owner->GetGUID());
    }
}
}

class spell_area52_corrupted_bear : public AuraScript
{
    PrepareAuraScript(spell_area52_corrupted_bear);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({99636, 99638, 954340, 172});
    }

    bool CheckProc(ProcEventInfo& event)
    {
        return Area52CorruptedBear::Active(GetTarget()) && event.GetActor() == GetTarget() &&
            event.GetActionTarget() && GetTarget()->IsValidAttackTarget(event.GetActionTarget()) &&
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() &&
            !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) &&
            (!event.GetProcSpell() || !event.GetProcSpell()->IsTriggered());
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* owner = GetTarget()->ToPlayer();
        Unit* target = event.GetActionTarget();
        SpellInfo const* spell = event.GetSpellInfo();
        if (!owner || !target)
            return;
        owner->CastSpell(target, 99638, true, nullptr, effect);
        uint32 const root = spell ? spell->GetFirstRankSpell()->Id : 0;
        if (root == 6807 || root == 33878)
        {
            uint32 known = 0;
            std::set<uint32> visited;
            for (uint32 rank = 172; rank && visited.insert(rank).second;
                rank = sSpellMgr->GetNextSpellInChain(rank))
                if (owner->HasSpell(rank))
                    known = rank;
            if (known)
                owner->CastSpell(target, known, true, nullptr, effect);
        }
        if (root == 779)
            for (auto const& entry : target->GetAppliedAuras())
                if (entry.second->GetBase()->GetCasterGUID() == owner->GetGUID() &&
                    entry.second->GetBase()->GetSpellInfo()->GetFirstRankSpell()->Id == 172)
                {
                    owner->CastSpell(target, 99636, true, nullptr, effect);
                    break;
                }
    }

    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        Area52CorruptedBear::Synchronize(GetTarget());
    }

    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(954340, GetTarget()->GetGUID());
        GetTarget()->RemoveAurasDueToSpell(300600, GetTarget()->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_corrupted_bear::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_corrupted_bear::Proc,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectApply += AuraEffectApplyFn(spell_area52_corrupted_bear::Apply,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_corrupted_bear::Remove,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_corrupted_bear_form : public AuraScript
{
    PrepareAuraScript(spell_area52_corrupted_bear_form);

    void Synchronize(AuraEffect const*, AuraEffectHandleModes)
    {
        Area52CorruptedBear::Synchronize(GetTarget());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_corrupted_bear_form::Synchronize,
            EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_corrupted_bear_form::Synchronize,
            EFFECT_0, SPELL_AURA_MOD_SHAPESHIFT, AURA_EFFECT_HANDLE_REAL);
    }
};
