DELETE FROM `spell_proc` WHERE `SpellId` = 909019;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
`DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(909019, 16, 3, 512, 0, 0, 65536, 1, 2, 3, 0, 6, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 909019 AND `ScriptName` = 'spell_area52_honed_ice';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (909019, 'spell_area52_honed_ice');
