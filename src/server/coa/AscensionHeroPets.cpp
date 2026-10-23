/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionHeroPets.h"
#include "AscensionRemovedSpellActionBars.h"
#include "Config.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include <array>
#include <set>
#if __has_include("AscensionFreePick.h")
#include "AscensionFreePick.h"
#endif

namespace AscensionHeroPets
{
namespace
{
bool Enabled = false;

struct PetPackage
{
    uint32 Parent;
    std::array<uint32, 6> Children;
};

constexpr std::array<PetPackage, 5> Packages = {{
    { 965200, { 1515, 883, 2641, 982, 1462 } },
    { 891, { 885, 889, 893, 109980 } },
    { 890, { 884, 887, 892, 109981 } },
    { 91634, { 91631, 91633, 91652, 109982 } },
    { 91606, { 91602, 91605, 91651, 109983 } }
}};

void Grant(Player* player)
{
    if (!Applies(player))
        return;
    for (uint32 spell : {6991u, 1106991u, 1356991u})
        if (player->HasSpell(spell))
            player->removeSpell(spell, SPEC_MASK_ALL, false);
    AscensionRemovedSpellActionBars::Reconcile(player, {6991, 1106991, 1356991});
    std::set<uint32> spells;
    for (PetPackage const& package : Packages)
        if (player->HasSpell(package.Parent))
            for (uint32 child : package.Children)
                if (child)
                    spells.insert(child);
#if __has_include("AscensionFreePick.h")
    AscensionFreePick::SpellLearningScope learning(player, spells);
#endif
    for (uint32 spell : spells)
        if (!player->HasSpell(spell))
            player->learnSpell(spell);
}

class Realm final : public WorldScript
{
public:
    Realm() : WorldScript("AscensionHeroPetRealm") { }

    void OnStartup() override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live";
    }
};

class Heroes final : public PlayerScript
{
public:
    Heroes() : PlayerScript("AscensionHeroPets",
        { PLAYERHOOK_ON_PLAYER_IS_CLASS, PLAYERHOOK_ON_BEFORE_GUARDIAN_INIT_STATS_FOR_LEVEL,
            PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_LEARN_SPELL, PLAYERHOOK_ON_FORGOT_SPELL }) { }

    void OnPlayerLogin(Player* player) override
    {
        Grant(player);
    }

    void OnPlayerLearnSpell(Player* player, uint32 spell) override
    {
        for (PetPackage const& package : Packages)
            if (package.Parent == spell)
                Grant(player);
    }

    void OnPlayerForgotSpell(Player* player, uint32 spell) override
    {
        if (!Applies(player))
            return;
        for (PetPackage const& package : Packages)
            if (package.Parent == spell)
                for (uint32 child : package.Children)
                    if (child)
                        player->removeSpell(child, SPEC_MASK_ALL, false);
    }

    Optional<bool> OnPlayerIsClass(Player const* player, Classes playerClass, ClassContext context) override
    {
        if (Applies(player) && ((context == CLASS_CONTEXT_PET && playerClass == CLASS_HUNTER) ||
            (context == CLASS_CONTEXT_PET_CHARM && playerClass == CLASS_WARLOCK)))
            return true;
        return std::nullopt;
    }

    void OnPlayerBeforeGuardianInitStatsForLevel(Player* player, Guardian* guardian, CreatureTemplate const*,
        PetType& petType) override
    {
        if (Applies(player) && guardian->IsPet())
            petType = guardian->ToPet()->getPetType();
    }
};
}

bool Applies(Player const* player)
{
    return Enabled && player && player->getClass() == CLASS_HERO;
}
}

void AddAscensionHeroPetScripts()
{
    new AscensionHeroPets::Realm();
    new AscensionHeroPets::Heroes();
}
