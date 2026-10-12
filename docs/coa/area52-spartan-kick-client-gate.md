# Spartan Kick client prerequisite repair

The replacement remains named Kick (9901766), with a separate 20-second cooldown and category 0.

The installed client required CasterAuraSpell 84445. That enchant is passive (Attributes 0x800001d0), has no area-aura effect or ignore-aurastate effect, and does not pass Aura::CanBeSentToClient. The client prerequisite therefore cannot use its server-only aura state reliably. Clear only the client prerequisite; the server spell_dbc requirement remains 84445 and the ownership handler still requires both the enchant and learned Kick.

Reported character Hyperion has both 1766 and 9901766 saved with specMask 255 and no saved cooldown for either. This rules out missing persisted ownership or a persisted cooldown, not all transient runtime conditions.

The regression fails on the previous patcher and passes after correction. It checks the client requirement is clear, server requirement remains enforced, independent cooldown remains 20000 ms/category 0, and original Kick is unchanged. In-game client acceptance remains to be tested; no simulated server cast can prove the client precheck.

Reviewed upstream main: b366fa8434d0a50a03c6d4cf0611eae37d99821e. Installed Area 52 patch-D SHA256: 86598748c5f2ed16ac4034afc29d82c64edd6e3407fc743619dc9ba3d238d1d4.
