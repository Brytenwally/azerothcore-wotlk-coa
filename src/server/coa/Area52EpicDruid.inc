/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

namespace Area52EpicDruid
{
constexpr uint32 Evergreen = 81501;
constexpr uint32 Lifebloom = 33763;
constexpr uint32 DruidOfRot = 81248;
constexpr uint32 CallFungarians = 81249;
constexpr uint32 Fungarian = 43286;
constexpr uint32 CloudOfRot = 81413;

int32 RotDamage(uint32 level, int32 base, int32 spellPower, float attackPower)
{
    return std::max<int32>(0, int32(float(base) + 1.25f * level +
        0.02f * std::max<int32>(0, spellPower) + 0.013f * std::max(0.0f, attackPower)));
}

bool EvergreenHeal(uint32 family, uint32 mask, uint32 type, bool heal, bool owned)
{
    return owned && heal && family == SPELLFAMILY_DRUID && (mask & 0x60u) &&
        (type & PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS) && !(type & PROC_FLAG_DONE_PERIODIC);
}

void RefreshLifebloom(Unit* target, ObjectGuid caster)
{
    Unit::AuraEffectList const& effects = target->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL);
    for (AuraEffect* effect : effects)
        if (effect->GetCasterGUID() == caster &&
            sSpellMgr->GetFirstSpellInChain(effect->GetId()) == Lifebloom)
            effect->GetBase()->RefreshDuration();
}
}

class spell_area52_evergreen : public AuraScript
{
    PrepareAuraScript(spell_area52_evergreen);

    bool Check(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        return player && AscensionMysticEnchant::Applies(player) && spell && event.GetProcTarget() &&
            Area52EpicDruid::EvergreenHeal(spell->SpellFamilyName, spell->SpellFamilyFlags[0],
                event.GetTypeMask(), event.GetHealInfo() != nullptr, event.GetActor() == GetTarget());
    }

    void Handle(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Area52EpicDruid::RefreshLifebloom(event.GetProcTarget(), GetTarget()->GetGUID());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_evergreen::Check);
        OnEffectProc += AuraEffectProcFn(spell_area52_evergreen::Handle, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class spell_area52_call_fungarians : public SpellScript
{
    PrepareSpellScript(spell_area52_call_fungarians);

    SpellCastResult Check()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !AscensionMysticEnchant::Applies(player))
            return SPELL_CAST_OK;
        if (!player->HasAura(Area52EpicDruid::DruidOfRot) || !player->HasActiveSpell(33831))
            return SPELL_FAILED_CASTER_AURASTATE;
        return player->GetPet() ? SPELL_FAILED_ALREADY_HAVE_SUMMON : SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_call_fungarians::Check);
    }
};

class aura_area52_cloud_of_rot : public AuraScript
{
    PrepareAuraScript(aura_area52_cloud_of_rot);

    void Amount(AuraEffect const*, int32& amount, bool& recalculate)
    {
        Unit* caster = GetCaster();
        Unit* owner = caster ? caster->GetOwner() : nullptr;
        Player* player = owner ? owner->ToPlayer() : nullptr;
        if (!player || !AscensionMysticEnchant::Applies(player) ||
            caster->GetEntry() != Area52EpicDruid::Fungarian ||
            (caster->GetUInt32Value(UNIT_CREATED_BY_SPELL) != Area52EpicDruid::CallFungarians &&
                caster->GetUInt32Value(UNIT_CREATED_BY_SPELL) != 281618))
            return;
        recalculate = false;
        amount = Area52EpicDruid::RotDamage(player->GetLevel(),
            GetSpellInfo()->Effects[EFFECT_0].BasePoints + 1,
            player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_NATURE),
            player->GetTotalAttackPowerValue(BASE_ATTACK));
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_cloud_of_rot::Amount,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
    }
};
