DELETE FROM `spell_script_names`
WHERE `spell_id` = 14177 AND `ScriptName` = 'spell_area52_cold_blood_activation';

DELETE FROM `spell_script_names`
WHERE `spell_id` = 14177 AND `ScriptName` = 'aura_area52_cold_blood_support';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (14177, 'spell_area52_cold_blood_activation');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (14177, 'aura_area52_cold_blood_support');

DELETE FROM `spell_script_names`
WHERE (`spell_id`, `ScriptName`) IN
    ((530920, 'aura_area52_freezing_blood_consumed'), (979163, 'aura_area52_freezing_blood_damage'));

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(530920, 'aura_area52_freezing_blood_consumed'),
(979163, 'aura_area52_freezing_blood_damage');

DELETE FROM `spell_proc` WHERE `SpellId` = 979135;

INSERT INTO `spell_proc`
(`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
 `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
 `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
(979135, 0x10, 0, 0, 0, 0, 0x10010, 1, 2, 0, 0, 0, 0, 0, 0);
