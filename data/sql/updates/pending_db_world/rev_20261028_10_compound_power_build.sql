-- Marksman build (Compound Power): the four Mystic Enchants of the build that were not connected.
-- Compound Power (979740) and Sniper (81166) get their scripts (Area52Marksman.inc); Munitions Expert, Syndicate
-- Sniper Shot and Compound Power's Compound Shot are spell grants (AscensionMysticSpellGrants.h).
-- Compound Power procs on every damaging spell; its script keeps Steady, Arcane, Multi-, Chimera and Compound Shot.
DELETE FROM `spell_proc` WHERE `SpellId` = 979740;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(979740, 0, 0, 0, 0, 0, 69888, 1, 2, 0, 0, 2, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_compound_power', 'spell_area52_compound_power_slice_and_dice', 'spell_area52_sniper');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(979740, 'spell_area52_compound_power'),
(-5171, 'spell_area52_compound_power_slice_and_dice'),
(81166, 'spell_area52_sniper');

-- Heavy Munitions and High Velocity Munitions replace each other.
DELETE FROM `spell_group` WHERE `id` = 2000979;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(2000979, 978717),
(2000979, 978718);
DELETE FROM `spell_group_stack_rules` WHERE `group_id` = 2000979;
INSERT INTO `spell_group_stack_rules` (`group_id`, `stack_rule`, `description`) VALUES
(2000979, 1, 'Munitions Expert: Heavy Munitions or High Velocity Munitions');
