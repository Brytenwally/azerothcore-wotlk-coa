class spell_area52_legendary_proc : public AuraScript
{
    PrepareAuraScript(spell_area52_legendary_proc);

    bool Load() override
    {
        Player* player = GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr;
        return player && AscensionFreePick::Applies(player);
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        return Area52Legendary::Accept(GetId(), player && AscensionFreePick::Applies(player),
            event.GetActor() == player && event.GetActionTarget() && event.GetActionTarget() != player,
            event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() > 0,
            (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0, spell && spell->IsAffectingArea());
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_legendary_proc::CheckProc);
    }
};

class area52_legendary_metadata : public GlobalScript
{
public:
    area52_legendary_metadata() : GlobalScript("area52_legendary_metadata") { }

    void OnLoadSpellCustomAttr(SpellInfo* info) override
    {
        if (info->Id != Area52Legendary::Exorcism && info->Id != Area52Legendary::Consecration)
            return;
        if (!sConfigMgr->GetOption<bool>("CoA.Enable", true) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            (sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0) & ~uint32(0x400)))
            return;
        if (info->Id == Area52Legendary::Exorcism)
            info->SpellFamilyFlags[1] |= 2;
        if (info->Id == Area52Legendary::Consecration)
            info->SpellFamilyFlags[0] |= 32;
    }
};

class spell_area52_purification_hit : public SpellScript
{
    PrepareSpellScript(spell_area52_purification_hit);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Legendary::Vulnerability});
    }

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        if (player && AscensionFreePick::Applies(player) && player->HasAura(Area52Legendary::Purification) &&
            GetHitUnit() && GetHitDamage() > 0)
            player->CastSpell(GetHitUnit(), Area52Legendary::Vulnerability, true);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_area52_purification_hit::Hit);
    }
};

class aura_area52_purification_ground : public AuraScript
{
    PrepareAuraScript(aura_area52_purification_ground);

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({Area52Legendary::Vulnerability});
    }

    void Apply(AuraEffect const* effect, AuraEffectHandleModes)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (player && AscensionFreePick::Applies(player) && player->HasAura(Area52Legendary::Purification))
            player->CastSpell(GetTarget(), Area52Legendary::Vulnerability, true, nullptr, effect);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_area52_purification_ground::Apply,
            EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_area52_zealot_hit : public SpellScript
{
    PrepareSpellScript(spell_area52_zealot_hit);

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !AscensionFreePick::Applies(player) || !GetHitUnit())
            return;
        bool const pvp = GetExplTargetUnit() && GetExplTargetUnit()->IsControlledByPlayer();
        SetHitDamage(Area52Legendary::ZealotAmount(GetHitDamage(), pvp));
        SetHitHeal(Area52Legendary::ZealotAmount(GetHitHeal(), pvp));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_zealot_hit::Hit);
    }
};
