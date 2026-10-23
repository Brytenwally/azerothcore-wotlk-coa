#include "Area52HopliteSupportRules.h"

class spell_area52_shattering_throw : public SpellScript
{
    PrepareSpellScript(spell_area52_shattering_throw);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({58567}); }
    bool Removed = false;
    bool Landed = false;
    void Before(SpellMissInfo miss)
    {
        Unit* target = GetHitUnit();
        if (!target || miss != SPELL_MISS_NONE)
            return;
        Landed = true;
        uint64 const mask = uint64(1) << MECHANIC_IMMUNE_SHIELD;
        for (auto const& pair : target->GetAppliedAuras())
            if (pair.second->GetBase()->GetSpellInfo()->GetAllEffectsMechanicMask() & mask)
            {
                Removed = true;
                break;
            }
        if (Removed)
            target->RemoveAurasWithMechanic(mask, AURA_REMOVE_BY_ENEMY_SPELL);
    }
    void Hit()
    {
        if (!Landed || Removed || !GetHitUnit())
            return;
        GetCaster()->CastCustomSpell(58567, SPELLVALUE_AURA_STACK, 5, GetHitUnit(), true);
        GetCaster()->ToPlayer()->ModifySpellCooldown(64382, -30000);
    }
    void Register() override
    {
        BeforeHit += BeforeSpellHitFn(spell_area52_shattering_throw::Before);
        AfterHit += SpellHitFn(spell_area52_shattering_throw::Hit);
    }
};

class area52_relentless_finishers : public AllSpellScript
{
public:
    area52_relentless_finishers() : AllSpellScript("area52_relentless_finishers",
        {ALLSPELLHOOK_ON_PREPARE, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellPrepare(Spell* spell, Unit* caster, SpellInfo const* info) override
    {
        if (!spell || spell->IsTriggered() ||
            !AscensionFreePick::Applies(caster->ToPlayer()))
            return;
        if (info->GetFirstRankSpell()->Id == 20243)
            if (Item* weapon = caster->ToPlayer()->GetWeaponForAttack(BASE_ATTACK, true))
                if (weapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON)
                    spell->SetScriptWeaponDamageMultiplier(0.8f);
        if (!info->NeedsComboPoints())
            return;
        for (uint32 id : {58423, 58422, 14179})
            if (Aura* aura = caster->GetAura(id))
            {
                spell->SetScriptValue(14179, uint32(Area52HopliteSupport::RelentlessChance(
                    aura->GetSpellInfo()->Effects[EFFECT_0].PointsPerComboPoint, caster->GetComboPoints())));
                spell->SetScriptValue(14180, id);
                break;
            }
    }

    void OnSpellHitResult(Spell* spell, Unit*, uint8 miss, uint32, uint32, bool) override
    {
        if (!spell || miss != SPELL_MISS_NONE || !AscensionFreePick::Applies(spell->GetCaster()->ToPlayer()))
            return;
        auto const chance = spell->GetScriptValue(14179);
        spell->SetScriptValue(14179, 0);
        AuraEffect const* source = spell->GetCaster()->GetAuraEffect(spell->GetScriptValue(14180), EFFECT_0);
        if (source && chance && roll_chance_i(chance))
        {
            spell->GetCaster()->CastSpell(spell->GetCaster(), 14181, true, nullptr, source);
            spell->GetCaster()->CastSpell(spell->GetCaster(), 14182, true, nullptr, source);
        }
    }
};

class spell_area52_hoplite_support_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_hoplite_support_proc);

    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }

    bool Validate(SpellInfo const* info) override
    {
        auto const index = info->Id == 812297 ? EFFECT_2 : info->Id == 912326 ? EFFECT_1 : EFFECT_0;
        return ValidateSpellInfo({info->Effects[index].TriggerSpell, 912328});
    }

    bool Check(ProcEventInfo& event)
    {
        Unit* owner = GetTarget();
        Unit* target = event.GetActionTarget();
        auto const* spell = event.GetSpellInfo();
        uint32 const root = spell ? spell->GetFirstRankSpell()->Id : 0;
        bool const actor = event.GetActor() == owner;
        bool const damage = event.GetDamageInfo() && event.GetDamageInfo()->GetDamage();
        bool const periodic = event.GetTypeMask() & (PROC_FLAG_DONE_PERIODIC | PROC_FLAG_TAKEN_PERIODIC);
        switch (GetId())
        {
            case 812297:
                return actor && root == 100;
            case 912326:
                return actor && damage && !periodic && spell && spell->DmgClass == SPELL_DAMAGE_CLASS_MELEE &&
                    root != 26654 && root != 12723;
            case 12317: case 13045: case 13046: case 13047: case 13048:
                return Area52HopliteSupport::Enrage(GetId(), root, actor, damage,
                    owner->HealthBelowPct(50), periodic);
            case 51701:
                return target && target != owner && owner->IsValidAttackTarget(target) &&
                    Area52HopliteSupport::Honor(GetId(), actor, damage, periodic, spell != nullptr);
            case 84445:
                return Area52HopliteSupport::Spartan(root, actor, target && target->IsPlayer(),
                    owner->HasAura(2457) || owner->HasAura(71) || owner->HasAura(997743));
            default:
                return false;
        }
    }

    void Handle(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (GetId() == 912326)
        {
            GetTarget()->CastCustomSpell(912327, SPELLVALUE_BASE_POINT0, -1000, GetTarget(), true, nullptr, effect);
            return;
        }
        Unit* target = GetId() == 51701 || GetId() == 84445 ? event.GetActionTarget() : GetTarget();
        if (target)
            GetTarget()->CastSpell(target, effect->GetSpellInfo()->Effects[effect->GetEffIndex()].TriggerSpell,
                true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_hoplite_support_proc::Check);
        OnEffectProc += AuraEffectProcFn(spell_area52_hoplite_support_proc::Handle,
            EFFECT_ALL, SPELL_AURA_PROC_TRIGGER_SPELL);
    }
};

class aura_area52_sweeping_swings : public AuraScript
{
    PrepareAuraScript(aura_area52_sweeping_swings);
    bool Load() override { return AscensionFreePick::Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({912328}); }
    void Apply(AuraEffect const*, AuraEffectHandleModes)
    {
        if ((GetId() == 912326 && GetTarget()->HasAura(12328)) ||
            (GetId() == 12328 && GetTarget()->HasAura(912326)))
            GetTarget()->CastSpell(GetTarget(), 912328, true);
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        GetTarget()->RemoveAurasDueToSpell(912328, GetTarget()->GetGUID());
    }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_sweeping_swings::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_sweeping_swings::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_sweeping_swings_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_sweeping_swings_damage);
    bool Load() override { return AscensionFreePick::Applies(GetCaster()->ToPlayer()); }
    void Hit()
    {
        if (GetHitUnit() && GetHitUnit()->IsCharmedOwnedByPlayerOrPlayer() && GetCaster()->HasAura(912326))
            SetHitDamage(CalculatePct(GetHitDamage(), 75));
    }
    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_sweeping_swings_damage::Hit);
    }
};
