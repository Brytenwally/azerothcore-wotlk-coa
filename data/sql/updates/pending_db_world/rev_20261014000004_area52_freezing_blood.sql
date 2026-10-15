DELETE FROM `spell_script_names`
WHERE `spell_id` = 14177 AND `ScriptName` = 'spell_area52_cold_blood_activation';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (14177, 'spell_area52_cold_blood_activation');

DELETE FROM `spell_script_names`
WHERE (`spell_id`, `ScriptName`) IN
    ((530920, 'aura_area52_freezing_blood_consumed'), (979163, 'aura_area52_freezing_blood_damage'));

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(530920, 'aura_area52_freezing_blood_consumed'),
(979163, 'aura_area52_freezing_blood_damage');

UPDATE `spell_proc`
SET `ProcFlags` = 0x10010, `SpellFamilyName` = 0, `SpellFamilyMask0` = 0,
    `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0, `SchoolMask` = 0x10,
    `SpellTypeMask` = 1, `SpellPhaseMask` = 2
WHERE `SpellId` = 979135;
