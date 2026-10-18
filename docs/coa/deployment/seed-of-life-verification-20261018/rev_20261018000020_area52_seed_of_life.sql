DELETE FROM `spell_proc` WHERE `SpellId` = 965937;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(965937, 8, 7, 80, 0, 0, 262144, 2, 2, 3, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (965939, 966753) AND `ScriptName` IN ('spell_area52_bloom', 'spell_area52_bloom_heal');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(965939, 'spell_area52_bloom'),
(966753, 'spell_area52_bloom_heal');

DELETE FROM `spell_bonus_data` WHERE `entry` = 966753;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(966753, 0.2, 0, 0.09, 0, 'Area 52 - Bloom');
