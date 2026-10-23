constexpr uint32 MysticExtractItem = 98463;
constexpr uint32 MysticRuneItem = 375250;
constexpr char ScrollProgressSetting[] = "core.freepick.mystic.scroll";
std::mutex ScrollRequestLock;
std::map<ObjectGuid, std::deque<WorldPacket>> ScrollRequests;

uint32 ScrollLevel(Player const* player)
{
    auto const* settings = player->FindPlayerSettings(ScrollProgressSetting);
    return settings && settings->size() > 1 ? std::max(1u, (*settings)[1].value) : 1u;
}

uint64 ScrollProgress(Player const* player)
{
    auto const* settings = player->FindPlayerSettings(ScrollProgressSetting);
    return settings && !settings->empty() ? (*settings)[0].value : 1u;
}

void SendScrollProgress(Player* player)
{
    WorldPacket packet(0x5FC, 12);
    packet << ScrollProgress(player) << ScrollLevel(player);
    player->SendDirectMessage(&packet);
}

uint32 ScrollExtractPrice(Player const* player)
{
    return std::min(5000u, ScrollLevel(player) * 200 + 1000);
}

Item* FindScroll(Player* player, uint8 bag, uint8 slot)
{
    if (!Player::IsInventoryPos(bag, slot))
        return nullptr;
    Item* item = player->GetItemByPos(bag, slot);
    return item && !item->IsInTrade() && item->GetOwnerGUID() == player->GetGUID() ? item : nullptr;
}

Entry const* DrawScroll()
{
    if (Data.BySpell.empty())
        return nullptr;
    auto chosen = Data.BySpell.begin();
    std::advance(chosen, urand(0, uint32(Data.BySpell.size() - 1)));
    return &chosen->second;
}

bool ReplaceMysticScroll(Player* player, Item* source, uint32 entry, uint32 runes, uint32 money)
{
    if (!source || source->IsInTrade() || !sObjectMgr->GetItemTemplate(entry) ||
        player->GetItemCount(MysticRuneItem) < runes || player->GetMoney() < money)
        return false;
    std::unique_ptr<Item> reward(Item::CreateItem(entry, 1, player));
    if (!reward)
        return false;
    ItemPosCountVec dest;
    InventoryResult result = source->GetCount() == 1 ?
        player->CanStoreItem(source->GetBagSlot(), source->GetSlot(), dest, reward.get(), true) :
        player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, reward.get());
    if (result != EQUIP_ERR_OK)
    {
        player->SendEquipError(result, nullptr, nullptr, entry);
        return false;
    }
    uint32 one = 1;
    player->DestroyItemCount(source, one, true);
    if (runes)
        player->DestroyItemCount(MysticRuneItem, runes, true);
    if (money)
        player->ModifyMoney(-int32(money));
    Item* stored = player->StoreItem(dest, reward.release(), true);
    player->SendNewItem(stored, 1, true, false);
    auto transaction = CharacterDatabase.BeginTransaction();
    player->SaveInventoryAndGoldToDB(transaction);
    CharacterDatabase.CommitTransaction(transaction);
    return true;
}

char const* CollectMysticScroll(Player* player, State* state, Item* item, bool buyExtract)
{
    if (!CanChange(player, state))
        return "RE_DISENCHANT_UNKNOWN";
    if (!item || item->IsInTrade())
        return "RE_DISENCHANT_BAD_ITEM";
    auto found = Data.ByItem.find(item->GetEntry());
    if (found == Data.ByItem.end())
        return "RE_DISENCHANT_NOT_MYSTIC_SCROLL";
    uint32 const spell = found->second;
    if (state->Known.contains(spell))
        return "RE_DISENCHANT_ALREADY_KNOWN_ENCHANTMENT";
    if (!NearAltar(player))
        return "RE_DISENCHANT_NO_MYSTIC_ALTAR";
    bool const heldExtract = player->HasItemCount(MysticExtractItem, 1);
    uint32 const price = heldExtract ? 0 : ScrollExtractPrice(player);
    if (!heldExtract && (!buyExtract || !player->HasItemCount(MysticRuneItem, price)))
        return "RE_DISENCHANT_NO_MONEY";
    auto transaction = CharacterDatabase.BeginTransaction();
    auto* statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_FREEPICK_MYSTIC_COLLECTION);
    statement->SetData(0, player->GetSession()->GetAccountId());
    statement->SetData(1, spell);
    transaction->Append(statement);
    uint32 one = 1;
    player->DestroyItemCount(item, one, true);
    player->DestroyItemCount(heldExtract ? MysticExtractItem : MysticRuneItem, heldExtract ? 1 : price, true);
    player->SaveInventoryAndGoldToDB(transaction);
    CharacterDatabase.CommitTransaction(transaction);
    state->Known.insert(spell);
    SendResult(player, 0x609, "RE_DISENCHANT_OK");
    WorldPacket learned(0x5FA, 4);
    learned << spell;
    player->SendDirectMessage(&learned);
    return nullptr;
}

void HandleScrollRequest(Player* player, WorldPacket& packet)
{
    auto* state = player->CustomData.Get<State>(StateKey);
    if (packet.GetOpcode() == 0x611)
    {
        char const* result = "RE_PURCHASE_UNKNOWN";
        ItemTemplate const* scroll = sObjectMgr->GetItemTemplate(992720);
        ItemPosCountVec destination;
        if (packet.size())
            result = "RE_PURCHASE_UNKNOWN";
        else if (!scroll)
            result = "RE_PURCHASE_ITEM_NOT_FOUND";
        else if (player->GetMoney() < scroll->BuyPrice)
            result = "RE_PURCHASE_NOT_ENOUGH_MONEY";
        else if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, destination, 992720, 1) != EQUIP_ERR_OK)
            result = "RE_PURCHASE_NOT_ENOUGH_SPACE";
        else if (!NearAltar(player))
            result = "RE_PURCHASE_NO_MYSTIC_ALTAR";
        else if (!CanChange(player, state))
            result = "RE_PURCHASE_NOT_WHILE_CASTING";
        else if (scroll->BuyPrice <= uint32(INT32_MAX))
            if (Item* item = player->StoreNewItem(destination, 992720, true))
            {
                player->ModifyMoney(-int32(scroll->BuyPrice));
                player->SendNewItem(item, 1, true, false);
                auto transaction = CharacterDatabase.BeginTransaction();
                player->SaveInventoryAndGoldToDB(transaction);
                CharacterDatabase.CommitTransaction(transaction);
                result = "RE_PURCHASE_OK";
            }
        SendResult(player, 0x612, result);
        return;
    }
    if (packet.GetOpcode() == 0x607)
    {
        char const* error = "RE_DISENCHANT_UNKNOWN";
        if (packet.size() == 3)
        {
            bool buy = packet.read<uint8>(0) != 0;
            error = CollectMysticScroll(player, state, FindScroll(player, packet.read<uint8>(1), packet.read<uint8>(2)), buy);
        }
        if (error)
            SendResult(player, 0x609, error);
        return;
    }
    if (packet.GetOpcode() == 0x733)
    {
        char const* result = "RE_PURCHASE_MYSTIC_EXTRACT_UNKNOWN";
        uint32 const price = ScrollExtractPrice(player);
        if (!packet.size() && CanChange(player, state) && NearAltar(player) &&
            !player->HasItemCount(MysticExtractItem, 1) && player->HasItemCount(MysticRuneItem, price))
        {
            if (player->AddItem(MysticExtractItem, 1))
            {
                player->DestroyItemCount(MysticRuneItem, price, true);
                auto transaction = CharacterDatabase.BeginTransaction();
                player->SaveInventoryAndGoldToDB(transaction);
                CharacterDatabase.CommitTransaction(transaction);
                result = "RE_PURCHASE_MYSTIC_EXTRACT_OK";
            }
        }
        SendResult(player, 0x734, result);
        return;
    }
    char const* result = "RE_REFORGE_UNKNOWN";
    uint32 spell = 0;
    Item* item = packet.size() == 2 ? FindScroll(player, packet.read<uint8>(0), packet.read<uint8>(1)) : nullptr;
    if (!CanChange(player, state))
        result = "RE_REFORGE_NOT_WHILE_CASTING";
    else if (!item)
        result = "RE_REFORGE_BAD_ITEM";
    else if (item->GetEntry() != 992720 && !Data.ByItem.contains(item->GetEntry()))
        result = "RE_REFORGE_NOT_MYSTIC_SCROLL";
    else if (!NearAltar(player))
        result = "RE_REFORGE_NO_MYSTIC_ALTAR";
    else if (player->HasSpellCooldown(93235))
        result = "RE_REFORGE_UNKNOWN";
    else
    {
        uint32 runes = sConfigMgr->GetOption<uint32>("CoA.FreePickMysticEnchants.ReforgeRuneCost", 250);
        uint32 money = sConfigMgr->GetOption<uint32>("CoA.FreePickMysticEnchants.ReforgeMoneyCost", 25000);
        if (runes && player->HasItemCount(MysticRuneItem, runes))
            money = 0;
        else if (money && player->GetMoney() >= money)
            runes = 0;
        else
            result = "RE_REFORGE_NO_MONEY";
        if (std::string_view(result) != "RE_REFORGE_NO_MONEY")
            if (Entry const* target = DrawScroll())
                if (ReplaceMysticScroll(player, item, target->Item, runes, money))
                {
                    spell = target->Spell;
                    result = "RE_REFORGE_OK";
                    uint64 progress = ScrollProgress(player);
                    uint32 level = ScrollLevel(player);
                    uint64 gain = target->Quality == 2 ? 60 : target->Quality == 3 ? 80 : target->Quality == 4 ? 100 : 200;
                    for (uint32 aura : {85856u, 85857u})
                        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(aura); info && player->HasAura(aura))
                            gain = uint64(float(gain) * (float(info->Effects[EFFECT_1].BasePoints + 1) / 100.0f + 1.0f));
                    progress = std::min<uint64>(UINT32_MAX, progress + gain);
                    auto threshold = [](uint32 value) -> uint64
                    {
                        return value > 249 ? uint32(value * 0x1001 - 0x72038) : uint64(value * 7.5 * value + value * 354.0);
                    };
                    uint32 const oldLevel = level;
                    while (level < 1000000 && threshold(level) <= progress)
                        ++level;
                    player->UpdatePlayerSetting(ScrollProgressSetting, 0, uint32(progress));
                    player->UpdatePlayerSetting(ScrollProgressSetting, 1, level);
                    if (level > oldLevel)
                        player->AddItem(MysticExtractItem, level - oldLevel);
                    player->AddSpellCooldown(93235, 0, 1100, true);
                    player->SaveToDB(false, false);
                }
    }
    WorldPacket response(0x604, 64);
    response << result << spell;
    player->SendDirectMessage(&response);
    if (spell)
        SendScrollProgress(player);
}

bool QueueScrollRequest(WorldSession* session, WorldPacket const& packet)
{
    Player* player = session ? session->GetPlayer() : nullptr;
    if (!player || !Applies(player))
        return false;
    std::lock_guard<std::mutex> lock(ScrollRequestLock);
    auto& queue = ScrollRequests[player->GetGUID()];
    if (queue.size() < 16)
        queue.push_back(packet);
    return true;
}

void ProcessScrollRequests(Player* player)
{
    std::deque<WorldPacket> pending;
    {
        std::lock_guard<std::mutex> lock(ScrollRequestLock);
        auto found = ScrollRequests.find(player->GetGUID());
        if (found != ScrollRequests.end())
        {
            pending.swap(found->second);
            ScrollRequests.erase(found);
        }
    }
    for (auto& packet : pending)
        HandleScrollRequest(player, packet);
}
