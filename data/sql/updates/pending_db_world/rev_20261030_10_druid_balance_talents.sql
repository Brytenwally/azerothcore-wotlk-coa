-- Earth and Moon: Starsurge applies the debuff too, as the talent's tooltip says
UPDATE `spell_proc` SET `SpellFamilyMask2` = 524288 WHERE `SpellId` = -48506;

-- Celestial Focus rank 3: the capstone procs when the druid is hit by an interrupt
DELETE FROM `spell_proc` WHERE `SpellId` = 16924;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(16924, 0, 0, 0, 0, 0, 139808, 7, 2, 0, 0, 0, 0, 0, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 16924 AND `ScriptName` = 'aura_ascension_celestial_focus';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(16924, 'aura_ascension_celestial_focus');
