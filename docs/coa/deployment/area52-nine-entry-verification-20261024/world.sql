DELETE FROM `spell_script_names` WHERE `spell_id` = 820337 AND `ScriptName` = 'spell_area52_heart_crusader_damage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(820337, 'spell_area52_heart_crusader_damage');
DELETE FROM `spell_bonus_data` WHERE `entry` = 820337;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(820337, 0.2, 0, 0.1, 0, 'Area 52 Heart of the Crusader client formula');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_improved_consecration' AND `spell_id` IN (26573, 20116, 20922, 20923, 20924, 27173, 48818, 48819, 926574, 926576, 926578, 926580, 926582, 926584, 926586, 926588);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(26573, 'spell_area52_improved_consecration'),
(20116, 'spell_area52_improved_consecration'),
(20922, 'spell_area52_improved_consecration'),
(20923, 'spell_area52_improved_consecration'),
(20924, 'spell_area52_improved_consecration'),
(27173, 'spell_area52_improved_consecration'),
(48818, 'spell_area52_improved_consecration'),
(48819, 'spell_area52_improved_consecration'),
(926574, 'spell_area52_improved_consecration'),
(926576, 'spell_area52_improved_consecration'),
(926578, 'spell_area52_improved_consecration'),
(926580, 'spell_area52_improved_consecration'),
(926582, 'spell_area52_improved_consecration'),
(926584, 'spell_area52_improved_consecration'),
(926586, 'spell_area52_improved_consecration'),
(926588, 'spell_area52_improved_consecration');
DELETE FROM `spell_threat` WHERE `entry` = 410003;
INSERT INTO `spell_threat` (`entry`, `flatMod`, `pctMod`, `apPctMod`) VALUES (410003, 0, 1.85, 0);
DELETE FROM `spell_group` WHERE `id` = 1059 AND `spell_id` = 410003;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES (1059, 410003);
