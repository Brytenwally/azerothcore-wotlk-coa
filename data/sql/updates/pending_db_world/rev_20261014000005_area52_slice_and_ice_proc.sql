UPDATE `spell_proc`
SET `SchoolMask` = 16,
    `SpellFamilyName` = 0,
    `SpellFamilyMask0` = 0,
    `SpellFamilyMask1` = 0,
    `SpellFamilyMask2` = 0,
    `ProcFlags` = 65536,
    `SpellTypeMask` = 1,
    `SpellPhaseMask` = 2
WHERE `SpellId` = 978302;
