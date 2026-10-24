#include "Area52HopliteRemainingRules.h"

namespace HopliteRules = Area52HopliteRemaining;

template <class Predicate>
bool HopliteInteractionMatches(Player const* player, SpellInfo const* spell, Predicate accepts)
{
    if (!spell)
        return false;
    if (accepts(spell))
        return true;
    if (player)
        for (uint32 original : player->GetSpellInteractionOrigins(spell->Id))
            if (SpellInfo const* origin = sSpellMgr->GetSpellInfo(original); origin && accepts(origin))
                return true;
    return false;
}

class area52_hoplite_remaining_metadata : public GlobalScript
{
public:
    area52_hoplite_remaining_metadata() : GlobalScript("area52_hoplite_remaining_metadata") { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        if (info->Id == HopliteRules::HungerAbility)
            info->Effects[EFFECT_1].TriggerSpell = HopliteRules::HungerBuff;
        if (info->Id == HopliteRules::HungerBuff)
        {
            auto& pvp = info->Effects[EFFECT_1];
            pvp.ApplyAuraName = SPELL_AURA_ASCENSION_MOD_PVP_DAMAGE_DONE_PCT;
            pvp.MiscValue = 9;
            pvp.BasePoints = 4;
            pvp.SpellClassMask = flag96();
            info->Effects[EFFECT_2].TriggerSpell = HopliteRules::HungerSpeedBuff;
        }
        if (info->Id == HopliteRules::HungerSpeedBuff)
        {
            auto& speed = info->Effects[EFFECT_0];
            speed.ApplyAuraName = SPELL_AURA_MOD_INCREASE_SPEED;
            speed.MiscValue = 0;
            speed.BasePoints = 14;
            speed.SpellClassMask = flag96();
        }
        if (info->Id == HopliteRules::WindSlamEnchant)
        {
            auto& chance = info->Effects[EFFECT_1];
            chance.Effect = SPELL_EFFECT_APPLY_AURA;
            chance.ApplyAuraName = SPELL_AURA_ADD_FLAT_MODIFIER;
            chance.MiscValue = SPELLMOD_CHANCE_OF_SUCCESS;
            chance.BasePoints = 9;
            chance.DieSides = 1;
            chance.SpellClassMask = flag96(0, 0, 256);
            chance.TargetA = SpellImplicitTargetInfo(TARGET_UNIT_CASTER);
        }
        if (info->Id == HopliteRules::WindSlamBuff)
            info->Effects[EFFECT_1].Effect = 0;
        if (info->Id == HopliteRules::DraconicSinister)
        {
            auto& proc = info->Effects[EFFECT_0];
            proc.ApplyAuraName = SPELL_AURA_DUMMY;
            proc.MiscValue = 0;
            proc.TriggerSpell = HopliteRules::DraconicProc;
            info->Effects[EFFECT_1].Effect = 0;
        }

    }
};

class spell_area52_hunger_for_blood : public SpellScript
{
    PrepareSpellScript(spell_area52_hunger_for_blood);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({HopliteRules::HungerBuff, HopliteRules::HungerSpeedBuff}); }
    uint32 Count(Unit* target, bool own)
    {
        std::set<uint32> bleeds;
        if (target)
            for (auto const& pair : target->GetAppliedAuras())
            {
                Aura const* aura = pair.second->GetBase();
                if ((!own || aura->GetCasterGUID() == GetCaster()->GetGUID()) &&
                    (aura->GetSpellInfo()->GetAllEffectsMechanicMask() & (uint64(1) << MECHANIC_BLEED)))
                    bleeds.insert(aura->GetId());
            }
        return bleeds.size();
    }
    SpellCastResult Check()
    {
        return Count(GetExplTargetUnit(), false) ? SPELL_CAST_OK : SPELL_FAILED_TARGET_AURASTATE;
    }
    void Hit(SpellEffIndex)
    {
        uint32 const count = Count(GetHitUnit(), true);
        if (!count)
            return;
        Unit* caster = GetCaster();
        caster->CastSpell(caster, HopliteRules::HungerBuff, true);
        if (Aura* aura = caster->GetAura(HopliteRules::HungerBuff, caster->GetGUID()))
        {
            aura->SetMaxDuration(Area52HopliteRemaining::HungerDuration(count));
            aura->SetDuration(aura->GetMaxDuration());
        }
        caster->CastCustomSpell(HopliteRules::HungerSpeedBuff, SPELLVALUE_BASE_POINT0,
            Area52HopliteRemaining::HungerSpeed(count), caster, true);
        if (Aura* aura = caster->GetAura(HopliteRules::HungerSpeedBuff, caster->GetGUID()))
        {
            aura->SetMaxDuration(5000);
            aura->SetDuration(5000);
        }
    }
    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_area52_hunger_for_blood::Check);
        OnEffectHitTarget += SpellEffectFn(spell_area52_hunger_for_blood::Hit, EFFECT_1, SPELL_EFFECT_DUMMY);
    }
};

class aura_area52_wind_slam : public AuraScript
{
    PrepareAuraScript(aura_area52_wind_slam);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({HopliteRules::WindSlamEnchant, HopliteRules::WindSlamBuff, HopliteRules::WindSlamDamage}); }
    bool Check(ProcEventInfo& event)
    {
        auto const* spell = event.GetSpellInfo();
        if (!spell || event.GetActor() != GetTarget() || !event.GetDamageInfo() ||
            !event.GetDamageInfo()->GetDamage() || (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC))
            return false;
        if (GetId() == HopliteRules::WindSlamEnchant)
            return HopliteInteractionMatches(GetTarget()->ToPlayer(), spell, [](SpellInfo const* info)
                { return info->SpellFamilyName == SPELLFAMILY_SHAMAN && (info->SpellFamilyFlags[0] & 0x800000); });
        return GetTarget()->HasAura(HopliteRules::WindSlamEnchant) &&
            HopliteInteractionMatches(GetTarget()->ToPlayer(), spell, [](SpellInfo const* info)
                { return Area52HopliteRemaining::WindConsumer(info->GetFirstRankSpell()->Id); });
    }
    void Handle(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* owner = GetTarget();
        if (GetId() == HopliteRules::WindSlamEnchant)
        {
            owner->CastSpell(owner, HopliteRules::WindSlamBuff, true, nullptr, effect);
            return;
        }
        Unit* target = event.GetActionTarget();
        if (!target)
            return;
        bool const shield = HopliteInteractionMatches(owner->ToPlayer(), event.GetSpellInfo(), [](SpellInfo const* info)
            {
                uint32 const root = info->GetFirstRankSpell()->Id;
                return Area52HopliteRemaining::ShieldConsumer(root) || root == HopliteRules::LegacyShieldStrike;
            });
        owner->CastCustomSpell(HopliteRules::WindSlamDamage, SPELLVALUE_BASE_POINT1, shield ? -20 : 0, target, true, nullptr, effect);
        owner->RemoveAurasDueToSpell(HopliteRules::WindSlamBuff, owner->GetGUID());
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetId() == HopliteRules::WindSlamEnchant)
        {
            GetTarget()->RemoveAurasDueToSpell(HopliteRules::WindSlamBuff, GetTarget()->GetGUID());
            GetTarget()->RemoveAurasDueToSpell(HopliteRules::WindWithinBuff, GetTarget()->GetGUID());
        }
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_wind_slam::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_wind_slam::Handle, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_wind_slam::Remove,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_wind_slam_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_wind_slam_damage);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    void Slow(SpellEffIndex index)
    {
        if (!GetEffectValue())
            PreventHitDefaultEffect(index);
    }
    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_area52_wind_slam_damage::Slow, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
    }
};

class aura_area52_draconic_sinister : public AuraScript
{
    PrepareAuraScript(aura_area52_draconic_sinister);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({HopliteRules::DraconicSpellCrit, HopliteRules::DraconicWeaponCrit, HopliteRules::DraconicProc, HopliteRules::WorldforgedCooldown}); }
    bool Check(ProcEventInfo& event)
    {
        auto const* spell = event.GetSpellInfo();
        return spell && event.GetActor() == GetTarget() && event.GetDamageInfo() &&
            event.GetDamageInfo()->GetDamage() && !(event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) &&
            HopliteInteractionMatches(GetTarget()->ToPlayer(), spell, [](SpellInfo const* info)
                { return Area52HopliteRemaining::DraconicConsumer(info->GetFirstRankSpell()->Id); }) &&
            !GetTarget()->HasAura(HopliteRules::WorldforgedCooldown) &&
            roll_chance_i(sSpellMgr->GetSpellInfo(HopliteRules::DraconicProc)->ProcChance);
    }
    void Handle(AuraEffect const* effect, ProcEventInfo&)
    {
        PreventDefaultAction();
        int32 const bonus = effect->GetAmount();
        GetTarget()->CastCustomSpell(HopliteRules::DraconicSpellCrit, SPELLVALUE_BASE_POINT0, bonus, GetTarget(), true, nullptr, effect);
        GetTarget()->CastCustomSpell(HopliteRules::DraconicWeaponCrit, SPELLVALUE_BASE_POINT0, bonus, GetTarget(), true, nullptr, effect);
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        for (uint32 id : {HopliteRules::DraconicSpellCrit, HopliteRules::DraconicWeaponCrit})
            if (Aura* buff = GetTarget()->GetAura(id, GetTarget()->GetGUID()))
                if (auto const* source = buff->GetTriggeredByAuraSpellInfo(); source && source->Id == GetId())
                    buff->Remove();
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_draconic_sinister::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_draconic_sinister::Handle, EFFECT_0, SPELL_AURA_DUMMY);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_draconic_sinister::Remove,
            EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};
