/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AchievementCriteriaScript.h"
#include "AchievementScript.h"
#include "Config.h"
#include "DBCStructure.h"
#include <atomic>
#include "Player.h"
#include "ScriptMgr.h"

namespace
{
std::atomic<bool> RestrictSeasonal{false};

bool IsSeasonalAchievement(uint32 id)
{
    switch (id)
    {
        case 5006:
        case 5009:
        case 5012:
        case 5015:
        case 5016:
        case 16718:
        case 16719:
        case 16720:
        case 16721:
        case 16722:
        case 16723:
        case 16724:
        case 16904:
        case 16905:
        case 16906:
        case 16907:
        case 16908:
        case 16909:
        case 16910:
        case 17036:
            return true;
        default:
            return false;
    }
}

class SeasonalAchievementConfig final : public WorldScript
{
public:
    SeasonalAchievementConfig() : WorldScript("SeasonalAchievementConfig", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) { }

    void OnAfterConfigLoad(bool) override
    {
        RestrictSeasonal = sConfigMgr->GetOption<bool>("CoA.Enable", true)
            && sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "seasonal";
    }
};

class SeasonalAchievementCriteria final : public AchievementScript
{
public:
    SeasonalAchievementCriteria() : AchievementScript("SeasonalAchievementCriteria",
        { ACHIEVEMENTHOOK_CAN_CHECK_CRITERIA }) { }

    bool CanCheckCriteria(AchievementMgr*, AchievementCriteriaEntry const* criteria) override
    {
        return !RestrictSeasonal || !criteria || !IsSeasonalAchievement(criteria->referredAchievement);
    }
};

class SeasonalAchievementCompletion final : public PlayerScript
{
public:
    SeasonalAchievementCompletion() : PlayerScript("SeasonalAchievementCompletion",
        { PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE }) { }

    bool OnPlayerBeforeAchievementComplete(Player*, AchievementEntry const* achievement) override
    {
        return !RestrictSeasonal || !achievement || !IsSeasonalAchievement(achievement->ID);
    }
};

enum GameMode : uint32
{
    GAME_MODE_IRONMAN = 0x002,
    GAME_MODE_SURVIVALIST = 0x004,
    GAME_MODE_DRAFT = 0x008,
    GAME_MODE_RESOLUTE = 0x020,
    GAME_MODE_WILDCARD = 0x040,
    GAME_MODE_FELFORGED = 0x080,
    GAME_MODE_NIGHTMARE = 0x100
};

struct GameModeCriteria
{
    char const* Name;
    uint32 Modes;
};

constexpr GameModeCriteria GAME_MODE_CRITERIA[] = {
    { "achievement_coa_game_mode_ironman", GAME_MODE_IRONMAN },
    { "achievement_coa_game_mode_survivalist", GAME_MODE_SURVIVALIST },
    { "achievement_coa_game_mode_draft", GAME_MODE_DRAFT },
    { "achievement_coa_game_mode_resolute", GAME_MODE_RESOLUTE },
    { "achievement_coa_game_mode_ironman_resolute", GAME_MODE_IRONMAN | GAME_MODE_RESOLUTE },
    { "achievement_coa_game_mode_wildcard", GAME_MODE_WILDCARD },
    { "achievement_coa_game_mode_felforged", GAME_MODE_FELFORGED },
    { "achievement_coa_game_mode_nightmare", GAME_MODE_NIGHTMARE },
    { "achievement_coa_game_mode_ironman_nightmare", GAME_MODE_IRONMAN | GAME_MODE_NIGHTMARE },
    { "achievement_coa_game_mode_survivalist_nightmare", GAME_MODE_SURVIVALIST | GAME_MODE_NIGHTMARE },
    { "achievement_coa_game_mode_resolute_nightmare", GAME_MODE_RESOLUTE | GAME_MODE_NIGHTMARE },
    { "achievement_coa_game_mode_ironman_resolute_nightmare",
        GAME_MODE_IRONMAN | GAME_MODE_RESOLUTE | GAME_MODE_NIGHTMARE }
};

class AchievementGameModeCriteria final : public AchievementCriteriaScript
{
public:
    explicit AchievementGameModeCriteria(GameModeCriteria const& criteria)
        : AchievementCriteriaScript(criteria.Name), _modes(criteria.Modes) { }

    bool OnCheck(Player* player, Unit*, uint32) override
    {
        std::optional<uint32> const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
        return mask && (*mask & _modes) == _modes;
    }

private:
    uint32 _modes;
};
}

void AddAscensionAchievementConditionScripts()
{
    new SeasonalAchievementConfig();
    new SeasonalAchievementCriteria();
    new SeasonalAchievementCompletion();
    for (GameModeCriteria const& criteria : GAME_MODE_CRITERIA)
        new AchievementGameModeCriteria(criteria);
}
