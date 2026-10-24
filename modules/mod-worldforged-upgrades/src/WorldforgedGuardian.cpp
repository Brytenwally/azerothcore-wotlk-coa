/*
 * The Guardian of Time.
 *
 * Two gossip lines, one per store. Picking one sends SMSG_OPEN_CUSTOM_STORE
 * with the store id and the client opens its own RPGItemStore window; the list
 * inside it arrives when the client asks for it.
 *
 * Where he stood is not recovered. The client keeps the call board points of
 * interest, but only X and Y, so any spawn written here would be a guess. He is
 * created without one; place him with `.npc add 9780012`.
 */

#include "WorldforgedUpgrades.h"

#include "Creature.h"
#include "Config.h"
#include "Chat.h"
#include "WorldSession.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

namespace Worldforged
{
    void OpenStore(Player* player, uint32 store);
}

namespace
{
    bool Area52HeirloomShop(Player const* player)
    {
        if (!sConfigMgr->GetOption<bool>("Area52.GuardianHeirlooms.Enable", false) ||
            sConfigMgr->GetOption<std::string>("CoA.ClassModel", "coa") != "hero" ||
            sConfigMgr->GetOption<std::string>("CoA.RealmType", "live") != "live" ||
            !player || player->getClass() != CLASS_HERO)
            return false;
        auto const mask = sScriptMgr->OnPlayerGetGameModeMask(player);
        return !mask || !(*mask & ~uint32(0x400));
    }

    enum GossipAction
    {
        ACTION_WEAPONS = GOSSIP_ACTION_INFO_DEF + 1,
        ACTION_ARMOUR = GOSSIP_ACTION_INFO_DEF + 2,
        ACTION_HEIRLOOMS = GOSSIP_ACTION_INFO_DEF + 3,
        ACTION_HEIRLOOM_ARMOR = GOSSIP_ACTION_INFO_DEF + 4,
        ACTION_HEIRLOOM_WEAPONS = GOSSIP_ACTION_INFO_DEF + 5,
        ACTION_HEIRLOOM_ACCESSORIES = GOSSIP_ACTION_INFO_DEF + 6,
        ACTION_BACK = GOSSIP_ACTION_INFO_DEF + 7,
    };
}

class npc_worldforged_guardian : public CreatureScript
{
public:
    npc_worldforged_guardian() : CreatureScript("npc_worldforged_guardian") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        if (Area52HeirloomShop(player))
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Browse Heirlooms.",
                GOSSIP_SENDER_MAIN, ACTION_HEIRLOOMS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Upgrade Worldforged weapons.",
                         GOSSIP_SENDER_MAIN, ACTION_WEAPONS);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Upgrade Worldforged armor.",
                         GOSSIP_SENDER_MAIN, ACTION_ARMOUR);

        SendGossipMenuFor(player, Worldforged::GUARDIAN_ENTRY, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/,
                        uint32 action) override
    {
        ClearGossipMenuFor(player);

        switch (action)
        {
            case ACTION_BACK:
                return OnGossipHello(player, creature);
            case ACTION_HEIRLOOMS:
                if (!Area52HeirloomShop(player))
                    break;
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Armor (requires level 60).",
                    GOSSIP_SENDER_MAIN, ACTION_HEIRLOOM_ARMOR);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Weapons (requires level 60).",
                    GOSSIP_SENDER_MAIN, ACTION_HEIRLOOM_WEAPONS);
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "Accessories (requires level 60).",
                    GOSSIP_SENDER_MAIN, ACTION_HEIRLOOM_ACCESSORIES);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back.", GOSSIP_SENDER_MAIN, ACTION_BACK);
                SendGossipMenuFor(player, Worldforged::GUARDIAN_ENTRY, creature->GetGUID());
                return true;
            case ACTION_HEIRLOOM_ARMOR:
            case ACTION_HEIRLOOM_WEAPONS:
            case ACTION_HEIRLOOM_ACCESSORIES:
                if (!Area52HeirloomShop(player))
                    break;
                if (player->GetLevel() < 60)
                {
                    ChatHandler(player->GetSession()).SendSysMessage("You must be level 60 to buy heirlooms.");
                    return OnGossipHello(player, creature);
                }
                CloseGossipMenuFor(player);
                player->GetSession()->SendListInventory(creature->GetGUID(),
                    9781051 + action - ACTION_HEIRLOOM_ARMOR);
                return true;
            case ACTION_WEAPONS:
                Worldforged::OpenStore(player, Worldforged::STORE_WEAPONS);
                break;
            case ACTION_ARMOUR:
                Worldforged::OpenStore(player, Worldforged::STORE_ARMOUR);
                break;
            default:
                break;
        }

        CloseGossipMenuFor(player);
        return true;
    }
};

void AddWorldforgedGuardianScripts()
{
    new npc_worldforged_guardian();
}
