DELETE FROM `spell_proc` WHERE `SpellId` = 781046;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
    `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
    `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES (781046, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 0, 0, 0);
