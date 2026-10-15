UPDATE `spell_proc`
SET `SpellFamilyName` = 0,
    `SpellFamilyMask0` = 0,
    `SpellFamilyMask1` = 0,
    `SpellFamilyMask2` = 0,
    `ProcFlags` = 16,
    `SpellTypeMask` = 1,
    `SpellPhaseMask` = 2,
    `Chance` = 0
WHERE `SpellId` = 781046;
