UPDATE `spell_proc`
SET `SpellFamilyMask2` = `SpellFamilyMask2` | 0x20000000, `Cooldown` = 10000
WHERE `SpellId` IN (-63373, 63373, 63374, 263374);
