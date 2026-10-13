from pathlib import Path
from lupa import LuaRuntime
root = Path(__file__).resolve().parents[3]
source = (root / "docs/coa/area52-mystic-scrolls/Client-Scrolls.lua").read_text()
for context in (None, "MerchantFrame", "AuctionFrame", "AuctionHouseFrame", "TradeFrame", "MailFrame", "BankFrame"):
 lua = LuaRuntime()
 lua.execute("""
 shown=0
 function GetRealmName() return "Area 52 - Free-Pick" end
 function hooksecurefunc(name, fn) useHook=fn end
 C_MysticEnchant={GetMysticScrolls=function() return {{Bag=0,Slot=1,SpellID=42,Entry=123,Guid=456}} end,CanReforgeItem=function() return true end}
 EnchantCollectionUtil={ShowDisenchantItemDialogue=function() shown=shown+1 end}
 EnchantCollectionMixin={MYSTIC_ENCHANT_REFORGE_RESULT=function() end}
 """)
 if context:
  lua.execute(context+"={IsShown=function() return true end}")
 lua.execute(source)
 lua.execute("useHook(0,1)")
 assert lua.globals().shown == (0 if context else 1), context
 if context:
  lua.execute(context+".IsShown=function() return false end;useHook(0,1)")
  assert lua.globals().shown == 1, context
print("Scroll confirmation only opens in normal bag use; merchant, auction, trade, mail and bank contexts preserved")
