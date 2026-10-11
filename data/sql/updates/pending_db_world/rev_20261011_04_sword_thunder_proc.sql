DELETE FROM `spell_script_names` WHERE `spell_id` = 50227 AND `ScriptName` = 'spell_warr_sword_and_thunder_modifiers';
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-6343, 277079) AND `ScriptName` = 'spell_warr_sword_and_thunder_dispel';
DELETE FROM `spell_script_names` WHERE `spell_id` = 979777 AND `ScriptName` = 'spell_warr_sword_and_thunder_cleanup';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(50227, 'spell_warr_sword_and_thunder_modifiers'),
(-6343, 'spell_warr_sword_and_thunder_dispel'),
(277079, 'spell_warr_sword_and_thunder_dispel'),
(979777, 'spell_warr_sword_and_thunder_cleanup');
