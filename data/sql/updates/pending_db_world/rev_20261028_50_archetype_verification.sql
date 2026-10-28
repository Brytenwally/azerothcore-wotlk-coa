DELETE FROM `spell_bonus_data` WHERE `entry` = 954250;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(954250, 0.864, 0, 0.864, 0, 'Area 52 Dragon Roar client formula');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_archetype_cleanup';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(81166, 'aura_area52_archetype_cleanup'),
(979740, 'aura_area52_archetype_cleanup'),
(982890, 'aura_area52_archetype_cleanup'),
(954252, 'aura_area52_archetype_cleanup'),
(-53302, 'aura_area52_archetype_cleanup');

DELETE FROM `spell_group` WHERE `id` = 1058 AND `spell_id` = 179;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(1058, 179);
