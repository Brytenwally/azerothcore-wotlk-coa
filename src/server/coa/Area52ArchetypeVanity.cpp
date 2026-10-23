/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "Define.h"
#include "Area52ArchetypeVanityData.h"
#include "AscensionFreePick.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Item.h"
#include "ItemScript.h"
#include "Mail.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"
#include <algorithm>
#include <memory>

bool RedeemArea52ArchetypeVanity(Player* player, Item* item, std::vector<uint32> const& appearances, std::vector<uint32> const& rewards);

class item_area52_archetype_vanity : public ItemScript
{
public:
    item_area52_archetype_vanity() : ItemScript("item_area52_archetype_vanity") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (!AscensionFreePick::Applies(player) || !item)
            return true;
        auto const bundle = Area52ArchetypeVanity::Bundles.find(item->GetEntry());
        if (bundle != Area52ArchetypeVanity::Bundles.end())
            RedeemArea52ArchetypeVanity(player, item, bundle->second, Area52ArchetypeVanity::Items.at(item->GetEntry()));
        return true;
    }
};

class Area52ArchetypeVanityDelivery : public PlayerScript
{
public:
    Area52ArchetypeVanityDelivery() : PlayerScript("Area52ArchetypeVanityDelivery", {PLAYERHOOK_ON_LOGIN}) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!AscensionFreePick::Applies(player) || player->GetSession()->IsBot())
            return;
        uint32 const guid = player->GetGUID().GetCounter();
        if (!CharacterDatabase.Query("SELECT guid FROM area52_archetype_vanity_delivery WHERE guid = {} AND delivered_at = 0", guid))
            return;
        std::vector<std::unique_ptr<Item>> items;
        for (auto const& [entry, appearances] : Area52ArchetypeVanity::Bundles)
        {
            std::unique_ptr<Item> item(Item::CreateItem(entry, 1));
            if (!item)
                return;
            items.push_back(std::move(item));
        }
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        trans->Append("UPDATE area52_archetype_vanity_delivery SET delivered_at = {} WHERE guid = {} AND delivered_at = 0",
            uint32(GameTime::GetGameTime().count()), guid);
        for (std::size_t start = 0; start < items.size(); start += MAX_MAIL_ITEMS)
        {
            MailDraft draft("Area 52 Archetype Vanity Bundles", "One of each Archetype cosmetic bundle for testing. Open each to collect its wardrobe appearances.");
            auto const end = std::min(start + std::size_t(MAX_MAIL_ITEMS), items.size());
            for (auto i = start; i < end; ++i)
            {
                items[i]->SaveToDB(trans);
                draft.AddItem(items[i].release());
            }
            draft.SendMailTo(trans, MailReceiver(player), MailSender(player));
        }
        CharacterDatabase.CommitTransaction(trans);
    }
};

void AddSC_Area52ArchetypeVanity()
{
    new item_area52_archetype_vanity();
    new Area52ArchetypeVanityDelivery();
}
