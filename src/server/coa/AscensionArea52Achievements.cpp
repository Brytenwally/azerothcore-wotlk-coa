/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionArea52AchievementPolicy.h"
#include "AchievementMgr.h"
#include "AchievementScript.h"
#include "ClientDBC.h"
#include "Config.h"
#include "DBCStores.h"
#include "Log.h"
#include "ScriptMgr.h"
#include <atomic>

namespace
{
std::atomic<bool> Enabled{false};
std::set<uint32> Rejected;

bool Allows(uint32 id)
{
    return !Enabled || !Rejected.contains(id);
}
class Area52AchievementWorld final : public WorldScript
{
public:
    Area52AchievementWorld() : WorldScript("Area52AchievementWorld",
        { WORLDHOOK_ON_AFTER_CONFIG_LOAD, WORLDHOOK_ON_STARTUP }) { }

    void OnAfterConfigLoad(bool) override
    {
        Enabled = Area52Achievements::Enabled(sConfigMgr->GetOption<bool>("CoA.Enable", true),
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa"),
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live"),
            sConfigMgr->GetOption<uint32>("CoA.GameModeMask", 0),
            sConfigMgr->GetOption<bool>("CoA.FreePick.AchievementEligibility", true));
    }

    void OnStartup() override
    {
        ClientDBC achievements;
        ClientDBC criteria;
        if (!achievements.Load(GetClientDBCPath("Achievement.dbc"), 62) ||
            !criteria.Load(GetClientDBCPath("Achievement_Criteria.dbc"), 31))
        {
            LOG_ERROR("coa", "Area 52 achievement eligibility data could not be loaded; filter unavailable");
            return;
        }
        std::map<uint32, std::vector<std::vector<uint32>>> requirements;
        std::set<uint32> conditioned;
        for (uint32 i = 0; i < criteria.GetRecordCount(); ++i)
        {
            auto const row = criteria.GetRecord(i);
            std::vector<uint32> values;
            for (uint32 field = 2; field < 9; ++field)
                values.push_back(row.GetUInt32(field));
            for (uint32 field = 26; field < 31; ++field)
                values.push_back(row.GetUInt32(field));
            requirements[row.GetUInt32(1)].push_back(std::move(values));
            auto const* entry = sAchievementCriteriaStore.LookupEntry(row.GetUInt32(0));
            if (entry && sAchievementMgr->GetCriteriaDataSet(entry))
                conditioned.insert(row.GetUInt32(1));
        }
        std::map<uint32, uint32> categories;
        for (uint32 i = 0; i < sAchievementCategoryStore.GetNumRows(); ++i)
            if (auto const* category = sAchievementCategoryStore.LookupEntry(i))
                categories[category->ID] = category->parentCategory;
        std::vector<Area52Achievements::Record> records;
        for (uint32 i = 0; i < achievements.GetRecordCount(); ++i)
        {
            auto const row = achievements.GetRecord(i);
            auto const* entry = sAchievementStore.LookupEntry(row.GetUInt32(0));
            if (!entry)
                continue;
            Area52Achievements::Record record;
            record.Id = entry->ID;
            record.Category = entry->categoryId;
            record.Name = row.GetString(4);
            record.RealmFirst = (entry->flags & (ACHIEVEMENT_FLAG_REALM_FIRST_REACH | ACHIEVEMENT_FLAG_REALM_FIRST_KILL)) != 0;
            record.HasConditions = conditioned.contains(entry->ID) || entry->refAchievement != 0;
            record.HasRewards = sAchievementMgr->GetAchievementReward(entry) != nullptr;
            record.Criteria = requirements[entry->ID];
            for (uint32 field : {1u, 2u, 3u, 38u, 39u, 41u, 42u, 60u, 61u})
                record.Requirements.push_back(row.GetUInt32(field));
            records.push_back(std::move(record));
        }
        Rejected = Area52Achievements::Build(std::move(records), categories);
        LOG_INFO("coa", "Area 52 achievement eligibility: {} alternate-mode or duplicate records excluded; enabled={}",
            Rejected.size(), bool(Enabled));
    }
};
class Area52AchievementCriteria final : public AchievementScript
{
public:
    Area52AchievementCriteria() : AchievementScript("Area52AchievementCriteria",
        { ACHIEVEMENTHOOK_CAN_CHECK_CRITERIA }) { }
    bool CanCheckCriteria(AchievementMgr*, AchievementCriteriaEntry const* criteria) override
    {
        return !criteria || Allows(criteria->referredAchievement);
    }
};
class Area52AchievementCompletion final : public PlayerScript
{
public:
    Area52AchievementCompletion() : PlayerScript("Area52AchievementCompletion",
        { PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE }) { }
    bool OnPlayerBeforeAchievementComplete(Player*, AchievementEntry const* achievement) override
    {
        return !achievement || Allows(achievement->ID);
    }
};
}
void AddAscensionArea52AchievementScripts()
{
    new Area52AchievementWorld();
    new Area52AchievementCriteria();
    new Area52AchievementCompletion();
}
