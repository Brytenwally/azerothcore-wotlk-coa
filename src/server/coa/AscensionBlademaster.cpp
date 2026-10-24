/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Area52BlademasterRules.h"
#include "AscensionWildcard.h"
#include "Config.h"
#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Timer.h"
#include "WorldSession.h"
#include <array>
#include <limits>

namespace
{
constexpr uint32 Enchant = 965862;
constexpr uint32 Empowerment = 965863;
constexpr uint32 Study = 990042;
constexpr uint32 Shatter = 965865;
constexpr uint32 DivineForce = 965867;
constexpr uint32 Deflection = 965869;
constexpr uint32 Stagger = 20231;
constexpr uint32 StaggerDebt = 20232;
constexpr uint32 Riposte = 14251;
constexpr uint32 Devastate = 20243;
constexpr uint32 DivineStorm = 53385;
constexpr std::array<uint32, 3> Talents = {12295, 12676, 12677};
constexpr std::array<uint32, 3> Helpers = {812295, 812676, 812677};
constexpr std::array<uint32, 7> StormRanks = {53385, 54762, 54763, 54764, 54765, 54766, 54767};
bool Enabled = false;

bool Applies(Player const* player)
{
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

bool Eligible(Player* player)
{
    if (!Applies(player))
        return false;
    Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return Area52Blademaster::Eligible(main && main->GetTemplate()->Class == ITEM_CLASS_WEAPON,
        off && off->GetTemplate()->Class == ITEM_CLASS_WEAPON, player->HasSpell(46917),
        player->HasAura(53590) || player->HasAura(53591) || player->HasAura(53592),
        AscensionWildcard::PrimaryStat(player) == 1152, player->HasAura(978217));
}

void Synchronize(Player* player)
{
    if (!Applies(player) || !player->IsInWorld() || !player->IsAlive() || !player->GetSession() ||
        player->GetSession()->PlayerLogout())
        return;
    uint32 wanted = 0;
    if (Eligible(player))
        for (std::size_t i = 0; i < Talents.size(); ++i)
            if (player->HasAura(Talents[i]))
                wanted = Helpers[i];
    for (uint32 helper : Helpers)
        if (helper != wanted)
            player->RemoveAurasDueToSpell(helper, player->GetGUID());
    if (wanted && !player->HasAura(wanted))
        player->CastSpell(player, wanted, true);
    if (wanted == Helpers.back())
    {
        if (!player->HasAura(Stagger))
            player->CastSpell(player, Stagger, true);
    }
    else
    {
        player->RemoveAurasDueToSpell(Stagger, player->GetGUID());
        player->RemoveAurasDueToSpell(Deflection, player->GetGUID());
    }
}

class BlademasterWorld final : public WorldScript
{
public:
    BlademasterWorld() : WorldScript("Area52BlademasterWorld") { }
    void OnAfterConfigLoad(bool) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

class BlademasterContracts final : public GlobalScript
{
public:
    BlademasterContracts() : GlobalScript("Area52BlademasterContracts") { }
    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (!Enabled)
            return;
        if (info->Id == Enchant)
        {
            info->Effects[1].Effect = 0;
            info->Effects[2].Effect = 0;
        }
        if (std::find(Talents.begin(), Talents.end(), info->Id) != Talents.end())
            info->Effects[0].BasePoints = -1;
        if (info->Id == Study)
        {
            info->StackAmount = 3;
            info->Effects[0].ApplyAuraName = SPELL_AURA_DUMMY;
        }
        if (info->Id == Shatter || info->Id == DivineForce || info->Id == Deflection)
            info->StackAmount = 1;
        if (info->Id == Stagger)
        {
            info->Effects[0].ApplyAuraName = SPELL_AURA_SCHOOL_ABSORB;
            info->Effects[1].Effect = 0;
        }
        if (info->Id == StaggerDebt)
        {
            info->Effects[0].ApplyAuraName = SPELL_AURA_PERIODIC_DUMMY;
            info->Effects[0].Amplitude = 1000;
            info->Effects[1].Effect = 0;
            info->Effects[2].Effect = 0;
            info->AttributesEx4 |= SPELL_ATTR4_DAMAGE_DOESNT_BREAK_AURAS;
        }
        if (std::find(StormRanks.begin(), StormRanks.end(), info->Id) != StormRanks.end())
            info->MaxAffectedTargets = 6;
    }
};

class BlademasterPlayer final : public PlayerScript
{
public:
    BlademasterPlayer() : PlayerScript("Area52BlademasterPlayer") { }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override
    {
        if (!Applies(player))
            return;
        uint32 root = sSpellMgr->GetFirstSpellInChain(spell);
        if (root == DivineStorm)
            player->RemoveAurasDueToSpell(DivineForce, player->GetGUID());
        if (root == Riposte)
            player->RemoveAurasDueToSpell(Deflection, player->GetGUID());
    }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (!Applies(player))
            return;
        for (std::size_t i = 0; i < Talents.size(); ++i)
            if (player->HasAura(Talents[i]) || player->HasAura(Helpers[i]))
            {
                Synchronize(player);
                return;
            }
    }
};

class aura_area52_tactical_mastery : public AuraScript
{
    PrepareAuraScript(aura_area52_tactical_mastery);
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo(Helpers) && ValidateSpellInfo({Stagger}); }
    void Refresh(AuraEffect const*, AuraEffectHandleModes) { Synchronize(GetTarget()->ToPlayer()); }
    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_tactical_mastery::Refresh, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_tactical_mastery::Refresh, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class aura_area52_blademaster : public AuraScript
{
    PrepareAuraScript(aura_area52_blademaster);
    bool _hasCritical = false;
    uint32 _lastCritical = 0;
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Study, Empowerment, DivineForce, Deflection});
    }
    bool Check(ProcEventInfo& event)
    {
        if (event.GetActionTarget() == GetTarget() && (event.GetHitMask() & PROC_HIT_PARRY))
            return true;
        return event.GetActor() == GetTarget() && (event.GetHitMask() & PROC_HIT_CRITICAL) &&
            (event.GetTypeMask() & PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS) && event.GetSpellInfo() &&
            (!event.GetProcSpell() || !event.GetProcSpell()->IsTriggered()) &&
            (!_hasCritical || getMSTimeDiff(_lastCritical, getMSTime()) >= 8000);
    }
    void Proc(AuraEffect const*, ProcEventInfo& event)
    {
        PreventDefaultAction();
        if (event.GetActor() == GetTarget())
        {
            _hasCritical = true;
            _lastCritical = getMSTime();
        }
        Unit* owner = GetTarget();
        owner->CastSpell(owner, Study, true);
        if (Aura* stacks = owner->GetAura(Study, owner->GetGUID()); stacks && stacks->GetStackAmount() >= 3)
        {
            stacks->Remove();
            owner->CastSpell(owner, Empowerment, true);
        }
    }
    void Remove(AuraEffect const*, AuraEffectHandleModes)
    {
        for (uint32 id : {Study, Empowerment, DivineForce, Deflection})
            GetTarget()->RemoveAurasDueToSpell(id, GetTarget()->GetGUID());
    }
    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_blademaster::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_blademaster::Proc, EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_blademaster::Remove, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_blademaster_empower : public SpellScript
{
    PrepareSpellScript(spell_area52_blademaster_empower);
    bool _selfApplied = false;
    bool Load() override { return Applies(GetCaster()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({Shatter, DivineForce, Deflection}); }
    void Hit()
    {
        Unit* caster = GetCaster();
        if (!caster->HasAura(Enchant) || !caster->HasAura(Empowerment) || GetHitDamage() <= 0)
            return;
        Player* player = caster->ToPlayer();
        uint32 const spell = GetSpellInfo()->Id;
        if (player->MatchesSpellInteraction(spell, Devastate))
            caster->CastSpell(GetHitUnit(), Shatter, true);
        else if (!_selfApplied)
        {
            _selfApplied = true;
            if (player->MatchesSpellInteraction(spell, DivineStorm))
                caster->CastSpell(caster, DivineForce, true);
            else if (player->MatchesSpellInteraction(spell, Riposte) && caster->HasAura(Stagger))
                caster->CastSpell(caster, Deflection, true);
        }
    }
    void Register() override { AfterHit += SpellHitFn(spell_area52_blademaster_empower::Hit); }
};

class aura_area52_tactical_stagger : public AuraScript
{
    PrepareAuraScript(aura_area52_tactical_stagger);
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({StaggerDebt, Deflection}); }
    void Calculate(AuraEffect const*, int32& amount, bool& recalculate)
    {
        amount = -1;
        recalculate = false;
    }
    void Absorb(AuraEffect*, DamageInfo& damage, uint32& absorb)
    {
        absorb = 0;
        Player* owner = GetTarget()->ToPlayer();
        if (!Eligible(owner) || !owner->HasAura(Talents.back()) || damage.GetDamageType() == DOT ||
            !damage.GetAttacker() || damage.GetAttacker() == owner)
            return;
        uint32 bonus = 0;
        if (AuraEffect const* effect = owner->GetAuraEffect(Deflection, EFFECT_0))
            bonus = std::max(0, effect->GetAmount());
        uint32 deferred = Area52Blademaster::DeferredDamage(damage.GetDamage(), bonus,
            damage.GetAttacker()->GetCharmerOrOwnerPlayerOrPlayerItself() != nullptr);
        if (!deferred)
            return;
        Aura* debt = owner->GetAura(StaggerDebt, owner->GetGUID());
        if (!debt)
        {
            owner->CastCustomSpell(StaggerDebt, SPELLVALUE_BASE_POINT0, 0, owner, true);
            debt = owner->GetAura(StaggerDebt, owner->GetGUID());
        }
        if (!debt || !debt->GetEffect(EFFECT_0))
            return;
        AuraEffect* pool = debt->GetEffect(EFFECT_0);
        uint32 old = std::max(0, pool->GetAmount());
        absorb = std::min(deferred, uint32(std::numeric_limits<int32>::max()) - old);
        pool->ChangeAmount(old + absorb);
        debt->SetDuration(debt->GetMaxDuration());
    }
    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_area52_tactical_stagger::Calculate,
            EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
        OnEffectAbsorb += AuraEffectAbsorbFn(aura_area52_tactical_stagger::Absorb, EFFECT_0);
    }
};

class aura_area52_tactical_debt : public AuraScript
{
    PrepareAuraScript(aura_area52_tactical_debt);
    bool Load() override { return Applies(GetUnitOwner()->ToPlayer()); }
    void Tick(AuraEffect const* effect)
    {
        uint32 debt = std::max(0, effect->GetAmount());
        uint32 ticks = 1 + std::max(0, GetDuration()) / 1000;
        uint32 payment = Area52Blademaster::DebtPayment(debt, ticks);
        GetEffect(EFFECT_0)->ChangeAmount(debt - payment);
        Unit* owner = GetTarget();
        if (payment && owner->IsAlive())
        {
            uint32 dealt = Unit::DealDamage(owner, owner, payment, nullptr, DOT,
                SPELL_SCHOOL_MASK_NORMAL, GetSpellInfo(), false);
            owner->SendSpellNonMeleeDamageLog(owner, GetSpellInfo(), dealt, SPELL_SCHOOL_MASK_NORMAL,
                0, 0, false, 0);
        }
    }
    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(aura_area52_tactical_debt::Tick,
            EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
    }
};
}

void AddAscensionBlademasterScripts()
{
    new BlademasterWorld();
    new BlademasterContracts();
    new BlademasterPlayer();
    RegisterSpellScript(aura_area52_tactical_mastery);
    RegisterSpellScript(aura_area52_blademaster);
    RegisterSpellScript(spell_area52_blademaster_empower);
    RegisterSpellScript(aura_area52_tactical_stagger);
    RegisterSpellScript(aura_area52_tactical_debt);
}
