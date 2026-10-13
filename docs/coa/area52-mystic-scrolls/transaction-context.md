# Scroll transaction context repair

The UseContainerItem post-hook previously opened the scroll confirmation even when the
native call sold the scroll or handed it to another interface. Skip the confirmation while
MerchantFrame, AuctionFrame/AuctionHouseFrame, TradeFrame, MailFrame or BankFrame is visible.
Normal bag use retains the existing collection confirmation and cost. The scroll_context
Lua regression exercises all six contexts, normal use, and returning from a closed frame.

Live audit: all 3,822 identified scroll templates have bonding/Flags/FlagsExtra zero. All
112 existing identified scroll instances were unbound and had zero duration. Thus no global
auction permission change is justified by this report. The exact failing scroll/error is
still needed if auctioning via dragging into the sell slot fails after the UI correction.
Unimbued scroll 992720 is a different, currently bound crafting input; do not silently change
its binding while repairing identified enchant scroll interactions.

Client candidate combines this fix with the pending Feed Pet tooltip removal. Not installed,
not released. No server restart is needed for this UI-only correction.
