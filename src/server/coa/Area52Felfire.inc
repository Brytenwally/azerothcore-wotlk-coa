namespace Area52Felfire
{
constexpr std::array<uint32, 4> Originals = {50796, 59170, 59171, 59172};
constexpr std::array<uint32, 4> Replacements = {944811, 944812, 944813, 944814};

void Synchronize(Player* player)
{
    if (!player || !AscensionFreePick::Applies(player) || !player->GetSession() ||
        player->GetSession()->PlayerLogout())
        return;
    bool const active = AscensionMysticEnchant::GrantsActive(player, 965315);
    for (std::size_t index = 0; index < Originals.size(); ++index)
    {
        uint32 const original = Originals[index];
        uint32 const replacement = Replacements[index];
        AscensionMysticEnchant::SetGrantedSpell(player, 965315, replacement, active && player->HasSpell(original));
        if (active && player->HasActiveSpell(original))
            player->SetTemporarySpellReplacement(original, replacement);
        else if (player->GetTemporarySpellReplacement(original) == replacement)
            player->SetTemporarySpellReplacement(original, 0);
    }
}
}

class spell_area52_felfire_equipment : public AuraScript
{
    PrepareAuraScript(spell_area52_felfire_equipment);

    bool Load() override
    {
        return AscensionFreePick::Applies(GetUnitOwner() ? GetUnitOwner()->ToPlayer() : nullptr);
    }

    bool Validate(SpellInfo const*) override
    {
        return ValidateSpellInfo(Area52Felfire::Originals) && ValidateSpellInfo(Area52Felfire::Replacements);
    }

    void Synchronize(AuraEffect const*, AuraEffectHandleModes)
    {
        Area52Felfire::Synchronize(GetTarget()->ToPlayer());
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_area52_felfire_equipment::Synchronize,
            EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_area52_felfire_equipment::Synchronize,
            EFFECT_1, SPELL_AURA_PROC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

class area52_felfire_player : public PlayerScript
{
public:
    area52_felfire_player() : PlayerScript("area52_felfire_player") { }

    void Refresh(Player* player, uint32 spell)
    {
        if (std::find(Area52Felfire::Originals.begin(), Area52Felfire::Originals.end(), spell) !=
            Area52Felfire::Originals.end())
            Area52Felfire::Synchronize(player);
    }

    void OnPlayerLearnSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
    void OnPlayerForgotSpell(Player* player, uint32 spell) override { Refresh(player, spell); }
};

class spell_area52_pure_chaos_bolt : public SpellScript
{
    PrepareSpellScript(spell_area52_pure_chaos_bolt);

    void Hit()
    {
        Player* player = GetCaster()->ToPlayer();
        Unit* target = GetHitUnit();
        if (!player || !AscensionFreePick::Applies(player) || !player->HasAura(965315) || !target)
            return;
        std::vector<std::pair<uint32, ObjectGuid>> absorbs;
        for (AuraType type : {SPELL_AURA_SCHOOL_ABSORB, SPELL_AURA_MANA_SHIELD})
            for (AuraEffect const* aura : target->GetAuraEffectsByType(type))
                if (aura->GetCasterGUID().IsPlayer())
                    absorbs.emplace_back(aura->GetId(), aura->GetCasterGUID());
        for (auto const& [id, caster] : absorbs)
            target->RemoveAurasDueToSpell(id, caster);
        player->CastSpell(target, 984996, true);
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_area52_pure_chaos_bolt::Hit);
    }
};

class area52_felfire_casts : public AllSpellScript
{
public:
    area52_felfire_casts() : AllSpellScript("area52_felfire_casts", {ALLSPELLHOOK_ON_CAST}) { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        Unit* target = spell ? spell->m_targets.GetUnitTarget() : nullptr;
        if (!AscensionFreePick::Applies(player) || !player->HasAura(965315) || !spell ||
            spell->IsTriggered() || spell->GetCastTime() <= 0 || !target ||
            target->IsControlledByPlayer() || !player->IsValidAttackTarget(target))
            return;
        uint32 const root = info->GetFirstRankSpell()->Id;
        if (player->HasAura(17958) && (root == 5676 || root == 29722 || root == 2948 || root == 133))
            return;
        for (uint32 id : Area52Felfire::Originals)
            player->ModifySpellCooldown(id, -1500);
        for (uint32 id : Area52Felfire::Replacements)
            player->ModifySpellCooldown(id, -1500);
    }
};

class spell_area52_felfire_emberstorm : public SpellScript
{
    PrepareSpellScript(spell_area52_felfire_emberstorm);

    void Cast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!AscensionFreePick::Applies(player) || !player->HasAura(965315))
            return;
        for (uint32 id : Area52Felfire::Replacements)
            player->ModifySpellCooldown(id, -3000);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_area52_felfire_emberstorm::Cast);
    }
};
