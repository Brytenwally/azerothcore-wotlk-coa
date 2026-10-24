/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "WorldSession.h"

namespace
{
constexpr uint32 SpartanKick = 9901766;
constexpr char OwnershipKey[] = "core.area52.spartan.kick";

bool Applies(Player* player)
{
    if (!player || player->getClass() != CLASS_HERO || !sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
        sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
        sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live")
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}

void Synchronize(Player* player)
{
    if (!Applies(player) || !player->IsInWorld() || !player->IsAlive() || !player->GetSession() ||
        player->GetSession()->PlayerLogout())
        return;
    bool const active = player->HasAura(84445) && player->HasActiveSpell(1766);
    bool const owned = player->GetPlayerSetting(OwnershipKey, 0).value != 0;
    if (active)
    {
        if (!player->HasSpell(SpartanKick))
        {
            player->learnSpell(SpartanKick);
            if (!player->HasSpell(SpartanKick))
                return;
            player->UpdatePlayerSetting(OwnershipKey, 0, 1);
        }
        if (player->GetTemporarySpellReplacement(1766) != SpartanKick)
        {
            uint32 const remaining = player->GetSpellCooldownDelay(1766);
            if (remaining > player->GetSpellCooldownDelay(SpartanKick))
                player->AddSpellCooldown(SpartanKick, 0, remaining, true);
            player->SetTemporarySpellReplacement(1766, SpartanKick);
        }
    }
    else
    {
        if (player->GetTemporarySpellReplacement(1766) == SpartanKick)
        {
            uint32 const remaining = player->GetSpellCooldownDelay(SpartanKick);
            if (player->HasSpell(1766) && remaining > player->GetSpellCooldownDelay(1766))
                player->AddSpellCooldown(1766, 0, remaining, true);
            player->SetTemporarySpellReplacement(1766, 0);
        }
        if (owned)
        {
            player->removeSpell(SpartanKick, SPEC_MASK_ALL, false);
            player->UpdatePlayerSetting(OwnershipKey, 0, 0);
        }
    }
}
}

class spartan_kick_player : public PlayerScript
{
public:
    spartan_kick_player() : PlayerScript("area52_spartan_kick_player") { }
    void OnPlayerUpdate(Player* player, uint32) override
    {
        if (player->HasAura(84445) || player->HasSpell(SpartanKick))
            Synchronize(player);
    }
};

class aura_area52_spartan_kick : public AuraScript
{
    PrepareAuraScript(aura_area52_spartan_kick);

    bool Load() override
    {
        return Applies(GetUnitOwner()->ToPlayer());
    }

    bool Validate(SpellInfo const*) override { return ValidateSpellInfo({1766, 84446, SpartanKick}); }

    bool Check(ProcEventInfo& event)
    {
        Unit* owner = GetTarget();
        Unit* target = event.GetActionTarget();
        SpellInfo const* spell = event.GetSpellInfo();
        return event.GetActor() == owner && target && owner->IsValidAttackTarget(target) &&
            spell && (spell->GetFirstRankSpell()->Id == 1766 || spell->Id == SpartanKick) &&
            (owner->HasAura(2457) || owner->HasAura(71) || owner->HasAura(997743));
    }

    void Handle(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(event.GetActionTarget(), 84446, true, nullptr, effect);
    }

    void Refresh(AuraEffect const*, AuraEffectHandleModes) { Synchronize(GetTarget()->ToPlayer()); }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(aura_area52_spartan_kick::Check);
        OnEffectProc += AuraEffectProcFn(aura_area52_spartan_kick::Handle,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL);
        AfterEffectApply += AuraEffectApplyFn(aura_area52_spartan_kick::Refresh,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_area52_spartan_kick::Refresh,
            EFFECT_0, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddAscensionSpartanKickScripts()
{
    RegisterSpellScript(aura_area52_spartan_kick);
    new spartan_kick_player();
}
