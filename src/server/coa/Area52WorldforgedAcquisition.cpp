/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GameObject.h"
#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"
#include <unordered_map>

namespace
{
constexpr uint32 Key = 97729;
constexpr uint32 Fragment = 97730;
constexpr uint32 HeroClass = 10;

std::unordered_map<uint32, uint32> const Rewards = {
    {990100, 100369}, {990101, 132623}, {990102, 132624}, {990103, 132635},
    {990104, 132595}, {990105, 132583}, {990106, 132690}, {990107, 132701},
    {990108, 132709}, {990109, 132702}, {990110, 132638}, {990111, 132598},
    {990112, 132432}, {990113, 132609}, {990114, 132431}, {990115, 132689},
    {990116, 132580}, {990117, 132724}, {990118, 132424}, {990119, 134409},
    {990120, 132713}, {990121, 132602}
};

bool Enabled()
{
    return sConfigMgr->GetOption<bool>("Area52.WorldforgedAcquisition.Enable", false);
}

bool Exchange(Player* player, uint32 costItem, uint32 costCount, uint32 reward)
{
    if (!player->IsAlive() || player->getClass() != HeroClass)
        return false;
    if (!player->HasItemCount(costItem, costCount, false))
    {
        ChatHandler(player->GetSession()).SendSysMessage(costItem == Key
            ? "You need a Worldforged Key to open this cache."
            : "You need five Worldforged Key Fragments to create a key.");
        return false;
    }
    ItemPosCountVec destination;
    InventoryResult result = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, destination, reward, 1);
    if (result != EQUIP_ERR_OK)
    {
        player->SendEquipError(result, nullptr, nullptr, reward);
        return false;
    }
    Item* item = player->StoreNewItem(destination, reward, true);
    if (!item)
        return false;
    item->SetBinding(true);
    player->DestroyItemCount(costItem, costCount, true);
    auto transaction = CharacterDatabase.BeginTransaction();
    player->SaveInventoryAndGoldToDB(transaction);
    CharacterDatabase.CommitTransaction(transaction);
    player->SendNewItem(item, 1, true, false);
    return true;
}

class Cache final : public GameObjectScript
{
public:
    Cache() : GameObjectScript("area52_worldforged_scroll_cache") { }

    bool OnGossipHello(Player* player, GameObject* object) override
    {
        if (!Enabled() || !object || !player->InSamePhase(object) || player->GetExactDist(object) > 5.0f)
            return true;
        auto const found = Rewards.find(object->GetEntry());
        if (found != Rewards.end())
            Exchange(player, Key, 1, found->second);
        return true;
    }
};

class Fragments final : public ItemScript
{
public:
    Fragments() : ItemScript("item_area52_worldforged_fragments") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const&) override
    {
        if (!Enabled())
            return false;
        if (item->GetEntry() == Fragment)
            Exchange(player, Fragment, 5, Key);
        return true;
    }
};
}

void AddSC_Area52WorldforgedAcquisition()
{
    new Cache();
    new Fragments();
}
