DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_icy_penance_bolt', 'spell_area52_frozen_light_route', 'spell_area52_icy_spirit', 'aura_area52_icy_penance_channel');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(92762, 'spell_area52_frozen_light_route'),
(92766, 'spell_area52_icy_spirit'),
(92754, 'aura_area52_icy_penance_channel'),
(92755, 'aura_area52_icy_penance_channel'),
(92756, 'aura_area52_icy_penance_channel'),
(92757, 'aura_area52_icy_penance_channel');

DELETE FROM `spell_script_names` WHERE `spell_id` = 86576 AND `ScriptName` = 'aura_area52_epic_equipment';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (86576, 'aura_area52_epic_equipment');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_cryotherapist_gate';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(92752, 'spell_area52_cryotherapist_gate'),
(92754, 'spell_area52_cryotherapist_gate'),
(92755, 'spell_area52_cryotherapist_gate'),
(92756, 'spell_area52_cryotherapist_gate'),
(92757, 'spell_area52_cryotherapist_gate'),
(302039, 'spell_area52_cryotherapist_gate'),
(302040, 'spell_area52_cryotherapist_gate'),
(302041, 'spell_area52_cryotherapist_gate'),
(302042, 'spell_area52_cryotherapist_gate');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(302039, 'aura_area52_icy_penance_channel'),
(302040, 'aura_area52_icy_penance_channel'),
(302041, 'aura_area52_icy_penance_channel'),
(302042, 'aura_area52_icy_penance_channel');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_cryotherapy';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (86577, 'aura_area52_cryotherapy');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_spirit_frost';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (92753, 'aura_area52_spirit_frost');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_fingers_of_light_consumption';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(44543, 'aura_area52_fingers_of_light_consumption'),
(44545, 'aura_area52_fingers_of_light_consumption');
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_frozen_light';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (92752, 'aura_area52_frozen_light');
