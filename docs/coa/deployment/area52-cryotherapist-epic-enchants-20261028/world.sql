DELETE FROM `spell_ranks` WHERE `first_spell_id` = 997221;
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(997221, 997221, 1),
(997221, 997222, 2),
(997221, 997223, 3),
(997221, 997224, 4),
(997221, 997225, 5),
(997221, 997226, 6),
(997221, 997227, 7),
(997221, 997228, 8),
(997221, 997229, 9),
(997221, 997230, 10),
(997221, 997231, 11);

DELETE FROM `spell_script_names` WHERE `spell_id` = 414032 AND `ScriptName` = 'aura_area52_stay_frosty';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (414032, 'aura_area52_stay_frosty');
