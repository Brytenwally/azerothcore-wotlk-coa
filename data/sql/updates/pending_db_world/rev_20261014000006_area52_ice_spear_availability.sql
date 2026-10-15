DELETE FROM `spell_script_names`
WHERE `spell_id` = 984937 AND `ScriptName` = 'aura_area52_slice_and_ice_availability';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (984937, 'aura_area52_slice_and_ice_availability');

DELETE FROM `spell_proc` WHERE `SpellId` = 984937;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
    `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
    `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES (984937, 0, 0, 0, 0, 0, 0x15510, 7, 1, 0, 0, 0, 0, 0, 0, 0);
