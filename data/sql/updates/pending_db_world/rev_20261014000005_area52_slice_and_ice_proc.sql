DELETE FROM `spell_proc` WHERE `SpellId` = 978302;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
    `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
    `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES (978302, 16, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 0, 0, 0, 0, 0);
