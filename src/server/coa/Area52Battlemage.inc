bool HasArea52BattlemageHandlers(uint32 spell)
{
    if (spell != 81258 || !sSpellMgr->GetSpellProcEntry(spell))
        return false;
    std::list<AuraScript*> auras;
    sScriptMgr->CreateAuraScripts(spell, auras);
    bool found = false;
    for (auto* script : auras)
    {
        found |= *script->_GetScriptName() == "spell_area52_battlemage";
        delete script;
    }
    if (!found)
        return false;
    for (uint32 first : {44425u, 2136u, 30455u})
    {
        std::set<uint32> visited;
        for (uint32 rank = first; rank && visited.insert(rank).second;
            rank = sSpellMgr->GetNextSpellInChain(rank))
        {
            if (!sSpellMgr->GetSpellInfo(rank))
                return false;
            std::list<SpellScript*> scripts;
            sScriptMgr->CreateSpellScripts(rank, scripts);
            found = false;
            for (auto* script : scripts)
            {
                found |= *script->_GetScriptName() == "spell_area52_battlemage_damage";
                delete script;
            }
            if (!found)
                return false;
        }
    }
    return true;
}

class spell_area52_battlemage : public AuraScript
{
    PrepareAuraScript(spell_area52_battlemage);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo({44425, 2136, 30455});
    }

    std::vector<uint32> KnownChoices(Player* player) const
    {
        std::vector<uint32> choices;
        for (uint32 first : {44425u, 2136u, 30455u})
        {
            uint32 known = 0;
            std::set<uint32> visited;
            for (uint32 rank = first; rank && visited.insert(rank).second;
                rank = sSpellMgr->GetNextSpellInChain(rank))
                if (player->HasSpell(rank))
                    known = rank;
            if (known)
                choices.push_back(known);
        }
        return choices;
    }

    bool CheckProc(ProcEventInfo& event)
    {
        Player* player = GetTarget()->ToPlayer();
        SpellInfo const* spell = event.GetSpellInfo();
        return player && AscensionFreePick::Applies(player) &&
            Area52Battlemage::Accept(event.GetActor() == player && event.GetActionTarget() &&
                player->IsValidAttackTarget(event.GetActionTarget()),
                event.GetDamageInfo() && event.GetDamageInfo()->GetDamage() > 0,
                spell && spell->SpellFamilyName == SPELLFAMILY_WARRIOR,
                spell && spell->HasAttribute(SPELL_ATTR0_ON_NEXT_SWING_NO_DAMAGE),
                (event.GetTypeMask() & PROC_FLAG_DONE_PERIODIC) != 0) && !KnownChoices(player).empty();
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Player* player = GetTarget()->ToPlayer();
        Unit* target = event.GetActionTarget();
        if (!player || !target)
            return;
        auto const choices = KnownChoices(player);
        if (!choices.empty())
            player->CastSpell(target, choices[urand(0, uint32(choices.size() - 1))], true, nullptr, effect);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_area52_battlemage::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_area52_battlemage::Proc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

class spell_area52_battlemage_damage : public SpellScript
{
    PrepareSpellScript(spell_area52_battlemage_damage);

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        SpellInfo const* source = GetSpell()->GetTriggeredByAuraSpellInfo();
        if (player && AscensionFreePick::Applies(player) && source && source->Id == 81258 && GetHitUnit())
            SetHitDamage(Area52Battlemage::Damage(GetHitDamage(), GetHitUnit()->IsControlledByPlayer()));
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_battlemage_damage::Hit);
    }
};
