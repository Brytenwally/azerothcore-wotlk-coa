-- Intensify Rage (enchant and talent) also cools down Dragon Roar, which replaces Death Wish.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_intensify_rage';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(946908, 'spell_area52_intensify_rage'),
(46908, 'spell_area52_intensify_rage'),
(46909, 'spell_area52_intensify_rage'),
(56924, 'spell_area52_intensify_rage'),
(1146908, 'spell_area52_intensify_rage'),
(1146909, 'spell_area52_intensify_rage'),
(1156924, 'spell_area52_intensify_rage');
