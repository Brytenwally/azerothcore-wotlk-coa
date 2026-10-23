/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionPlayerComboPoints.h"
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include <algorithm>

namespace AscensionPlayerComboPoints
{
namespace
{
bool Enabled = false;
constexpr char Setting[] = "area52.player_combo_points";

class Realm final : public WorldScript
{
public:
    Realm() : WorldScript("AscensionPlayerComboPointsRealm") { }

    void OnStartup() override
    {
        Enabled = sConfigMgr->GetOption<bool>("CoA.Enable", true) &&
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") == "hero" &&
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") == "live" &&
            sConfigMgr->GetOption<bool>("CoA.FreePick.PlayerComboPoints", true);
    }
};

class Heroes final : public PlayerScript
{
public:
    Heroes() : PlayerScript("AscensionPlayerComboPoints", { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_SAVE }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (Applies(player))
            player->SetPlayerComboPoints(player->IsAlive() ?
                std::min<uint32>(5, player->GetPlayerSetting(Setting, 0).value) : 0);
    }

    void OnPlayerSave(Player* player) override
    {
        if (Applies(player))
            player->UpdatePlayerSetting(Setting, 0, player->IsAlive() ? player->GetComboPoints() : 0);
    }
};
}

bool Applies(Unit const* unit)
{
    Player const* player = unit ? unit->ToPlayer() : nullptr;
    if (!Enabled || !player || player->getClass() != CLASS_HERO)
        return false;
    auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
    return !mask || !(*mask & ~uint32(0x400));
}
}

void AddAscensionPlayerComboPointsScripts()
{
    new AscensionPlayerComboPoints::Realm();
    new AscensionPlayerComboPoints::Heroes();
}
