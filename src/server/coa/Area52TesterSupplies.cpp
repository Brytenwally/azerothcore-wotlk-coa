/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */
#include "AscensionFreePick.h"
#include "Creature.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameTime.h"
#include "Item.h"
#include "ItemScript.h"
#include <algorithm>
#include <memory>
#include <vector>
#include "Mail.h"
#include "MailMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "WorldSession.h"

namespace
{
constexpr uint32 TesterCache = 9900052;
constexpr uint32 ExperiencePotion = 3818046;

void DeliverWarchest(Player* player)
{
    if (!AscensionFreePick::Applies(player) || player->GetSession()->IsBot())
        return;
    player->GetSession()->SendItemQuerySingleResponse(657112);
    uint32 const account = player->GetSession()->GetAccountId();
    if (!CharacterDatabase.Query("SELECT id FROM area52_account_gift_campaign WHERE id = 657112 AND enabled = 1"))
        return;
    if (CharacterDatabase.Query("SELECT account FROM area52_account_gift_delivery "
        "WHERE campaign = 657112 AND account = {}", account))
        return;
    std::unique_ptr<Item> item(Item::CreateItem(657112, 1, player));
    if (!item)
        return;
    uint32 const guid = player->GetGUID().GetCounter();
    uint32 const itemGuid = item->GetGUID().GetCounter();
    uint32 const mailId = sObjectMgr->GenerateMailID();
    uint32 const now = uint32(GameTime::GetGameTime().count());
    uint32 const expires = now + 90 * DAY;
    std::string const subject = "Fel Enchanted Warchest";
    std::string const body = "A gift for your Area 52 account. Enjoy your Fel Enchanted Warchest!";
    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
    trans->Append("INSERT INTO area52_account_gift_delivery "
        "(campaign, account, receiver, item_guid, mail_id, delivered_at) VALUES (657112, {}, {}, {}, {}, {})",
        account, guid, itemGuid, mailId, now);
    item->SaveToDB(trans);
    auto* mail = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL);
    mail->SetData(0, mailId);
    mail->SetData(1, uint8(MAIL_NORMAL));
    mail->SetData(2, int8(MAIL_STATIONERY_GM));
    mail->SetData(3, uint16(0));
    mail->SetData(4, uint32(0));
    mail->SetData(5, guid);
    mail->SetData(6, subject);
    mail->SetData(7, body);
    mail->SetData(8, true);
    mail->SetData(9, expires);
    mail->SetData(10, now);
    mail->SetData(11, uint32(0));
    mail->SetData(12, uint32(0));
    mail->SetData(13, uint8(MAIL_CHECK_MASK_HAS_BODY));
    trans->Append(mail);
    auto* attachment = CharacterDatabase.GetPreparedStatement(CHAR_INS_MAIL_ITEM);
    attachment->SetData(0, mailId);
    attachment->SetData(1, itemGuid);
    attachment->SetData(2, guid);
    trans->Append(attachment);
    auto result = CharacterDatabase.AsyncCommitTransaction(trans);
    if (!result.m_future.get())
        return;
    if (!CharacterDatabase.Query("SELECT account FROM area52_account_gift_delivery "
        "WHERE campaign = 657112 AND account = {} AND item_guid = {} AND mail_id = {}", account, itemGuid, mailId))
        return;
    auto delivered = std::make_unique<Mail>();
    delivered->messageID = mailId;
    delivered->messageType = MAIL_NORMAL;
    delivered->stationery = MAIL_STATIONERY_GM;
    delivered->mailTemplateId = 0;
    delivered->sender = 0;
    delivered->receiver = guid;
    delivered->subject = subject;
    delivered->body = body;
    delivered->expire_time = expires;
    delivered->deliver_time = now;
    delivered->money = 0;
    delivered->COD = 0;
    delivered->checked = MAIL_CHECK_MASK_HAS_BODY;
    delivered->state = MAIL_STATE_UNCHANGED;
    delivered->AddItem(itemGuid, 657112);
    sMailMgr->OnMailSent(guid);
    player->AddNewMailDeliverTime(now);
    player->AddMail(delivered.release());
    player->AddMItem(item.release());
}

class Area52TesterSupplies : public PlayerScript
{
public:
    Area52TesterSupplies() : PlayerScript("Area52TesterSupplies", {PLAYERHOOK_ON_LOGIN}) { }

    void OnPlayerLogin(Player* player) override
    {
        DeliverWarchest(player);
        if (!AscensionFreePick::Applies(player) || player->GetSession()->IsBot() ||
            !sConfigMgr->GetOption<bool>("Area52.TesterCache.Enabled", false))
            return;
        uint32 const guid = player->GetGUID().GetCounter();
        if (CharacterDatabase.Query("SELECT guid FROM area52_tester_cache_delivery WHERE guid = {}", guid))
            return;
        Item* item = Item::CreateItem(TesterCache, 1);
        if (!item)
            return;
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        trans->Append("INSERT INTO area52_tester_cache_delivery (guid, delivered_at) VALUES ({}, {})",
            guid, uint32(GameTime::GetGameTime().count()));
        item->SaveToDB(trans);
        MailDraft("Area 52 Experimental Supply Crate",
            "Unstable ideas. Unlimited possibilities. Supplies for your Area 52 alpha testing adventures.")
            .AddItem(item).SendMailTo(trans, MailReceiver(player), MailSender(player));
        CharacterDatabase.CommitTransaction(trans);
    }
};

class item_area52_tester_cache : public ItemScript
{
public:
    item_area52_tester_cache() : ItemScript("item_area52_tester_cache") { }

    bool OnUse(Player* player, Item* cache, SpellCastTargets const&) override
    {
        if (!AscensionFreePick::Applies(player))
            return true;
        std::pair<uint32, uint32> const rewards[] = {
            {375250, 100000}, {97866, 500}, {134993, 3}, {3648545, 1}, {3818046, 5},
            {190190, 1}, {101169, 1}, {339075, 1}, {1642991, 1}
        };
        std::vector<std::unique_ptr<Item>> items;
        for (auto const& [entry, count] : rewards)
        {
            auto const* proto = sObjectMgr->GetItemTemplate(entry);
            if (!proto)
                return true;
            uint32 remaining = count;
            while (remaining)
            {
                uint32 const stack = std::min(remaining, proto->GetMaxStackSize());
                std::unique_ptr<Item> item(Item::CreateItem(entry, stack));
                if (!item)
                    return true;
                items.push_back(std::move(item));
                remaining -= stack;
            }
        }
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        uint32 consumed = 1;
        player->DestroyItemCount(cache, consumed, true);
        std::vector<std::unique_ptr<Item>> overflow;
        for (auto& item : items)
        {
            ItemPosCountVec dest;
            uint32 const count = item->GetCount();
            if (player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item.get()) == EQUIP_ERR_OK)
            {
                Item* stored = player->StoreItem(dest, item.release(), true);
                sScriptMgr->OnPlayerStoreNewItem(player, stored, count);
                player->SendNewItem(stored, count, true, false);
            }
            else
                overflow.push_back(std::move(item));
        }
        uint32 money = player->ModifyMoney(10000000, false) ? 0 : 10000000;
        player->SaveInventoryAndGoldToDB(trans);
        for (std::size_t start = 0; start < overflow.size() || money; start += MAX_MAIL_ITEMS)
        {
            MailDraft draft("Area 52 Experimental Supplies", "Your experimental supply crate contents. Happy testing!");
            draft.AddMoney(money);
            money = 0;
            auto const end = std::min(start + std::size_t(MAX_MAIL_ITEMS), overflow.size());
            for (auto i = start; i < end; ++i)
            {
                overflow[i]->SaveToDB(trans);
                draft.AddItem(overflow[i].release());
            }
            draft.SendMailTo(trans, MailReceiver(player), MailSender(player));
        }
        CharacterDatabase.CommitTransaction(trans);
        return true;
    }
};

class Area52InnkeeperSupplies : public AllCreatureScript
{
public:
    Area52InnkeeperSupplies() : AllCreatureScript("Area52InnkeeperSupplies") { }

    bool CanCreatureGossipHello(Player* player, Creature* creature) override
    {
        if (!AscensionFreePick::Applies(player) || !creature->IsInnkeeper())
            return false;
        creature->SetNpcFlag(UNIT_NPC_FLAG_VENDOR);
        VendorItemData const* stock = creature->GetVendorItems();
        if (!stock || !stock->FindItemCostPair(ExperiencePotion, 0))
            sObjectMgr->AddVendorItem(creature->GetEntry(), ExperiencePotion, 0, 0, 0, false);
        if (!(creature->GetCreatureTemplate()->npcflag & UNIT_NPC_FLAG_VENDOR) && !creature->GetScriptId())
        {
            player->PrepareGossipMenu(creature, creature->GetGossipMenuId(), true);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Browse experimental supplies.", GOSSIP_SENDER_MAIN, 9900052);
            SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());
            return true;
        }
        return false;
    }

    bool CanCreatureGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (!AscensionFreePick::Applies(player) || !creature->IsInnkeeper() ||
            sender != GOSSIP_SENDER_MAIN || action != 9900052)
            return false;
        player->GetSession()->SendListInventory(creature->GetGUID());
        return true;
    }
};
}

void AddSC_Area52TesterSupplies()
{
    new item_area52_tester_cache();
    new Area52TesterSupplies();
    new Area52InnkeeperSupplies();
}
