bool PreparePrestigeReset(Player* player, std::uint32_t& specialization, std::string& error)
{
    auto const* state = player ? player->CustomData.Get<State>(StateKey) : nullptr;
    if (!player || !CanChange(player, state) || player->GetActiveSpec() >= player->GetSpecsCount())
    {
        error = "Your active enchant specialization is not unlocked or ready for Prestige.";
        return false;
    }
    specialization = player->GetActiveSpec();
    return true;
}

std::uint32_t ResetPrestigeSpecialization(Player* player)
{
    auto const before = ReadSlots(player);
    auto const key = Setting(player);
    std::uint32_t removed = 0;
    for (std::uint32_t slot = 0; slot < before.size(); ++slot)
        if (before[slot])
        {
            player->UpdatePlayerSetting(key, slot, 0);
            ++removed;
        }
    Synchronize(player);
    SendSlots(player);
    return removed;
}
