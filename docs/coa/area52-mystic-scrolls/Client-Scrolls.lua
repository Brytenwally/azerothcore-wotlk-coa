local function IsArea52ScrollRealm()
    local realm = GetRealmName and GetRealmName() or ""
    return string.find(realm, "Area 52", 1, true) ~= nil
end

local function ConfirmArea52Scroll(bag, slot)
    for _, name in ipairs({"MerchantFrame", "AuctionFrame", "AuctionHouseFrame", "TradeFrame", "MailFrame", "BankFrame"}) do
        local frame = _G[name]
        if frame and frame:IsShown() then return end
    end
    if not IsArea52ScrollRealm() or not C_MysticEnchant or not EnchantCollectionUtil then return end
    local entries = C_MysticEnchant.GetMysticScrolls()
    for _, scroll in ipairs(entries or {}) do
        if scroll.Bag == bag and scroll.Slot == slot and scroll.SpellID and scroll.SpellID ~= 0 then
            EnchantCollectionUtil:ShowDisenchantItemDialogue(scroll.Entry, scroll.Guid)
            return
        end
    end
end

if not Area52MysticScrollConfirmationInstalled then
    Area52MysticScrollConfirmationInstalled = true
    hooksecurefunc("UseContainerItem", ConfirmArea52Scroll)
end

local OriginalCanReforgeItem = C_MysticEnchant.CanReforgeItem
C_MysticEnchant.CanReforgeItem = function(guid)
    local allowed, reason = OriginalCanReforgeItem(guid)
    if not IsArea52ScrollRealm() or reason ~= "RE_REFORGE_WORLDFORGED_SCROLL" then
        return allowed, reason
    end
    if not C_MysticEnchant.HasNearbyMysticAltar() then return false, "RE_REFORGE_NO_MYSTIC_ALTAR" end
    if UnitCastingInfo("player") or UnitChannelInfo("player") then return false, "RE_REFORGE_NOT_WHILE_CASTING" end
    local runes, money = C_MysticEnchant.GetReforgeCost()
    if not ((runes and runes > 0 and GetItemCount(375250) >= runes) or
            (money and money > 0 and GetMoney() >= money)) then
        return false, "RE_REFORGE_NO_MONEY"
    end
    return true
end

local OriginalReforgeResult = EnchantCollectionMixin.MYSTIC_ENCHANT_REFORGE_RESULT
function EnchantCollectionMixin:MYSTIC_ENCHANT_REFORGE_RESULT(result, ...)
    OriginalReforgeResult(self, result, ...)
    if IsArea52ScrollRealm() and result ~= "RE_REFORGE_OK" then
        self.waitingForReforge = false
        local tab = self:GetReforgeTab()
        tab:StopReforgeAnim()
        BaseFrameFadeIn(tab.AnimatedEnchant)
        tab:UpdateButtons()
    end
end
