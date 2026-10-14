DELETE FROM `spell_script_names`
WHERE `spell_id` = 978202 AND `ScriptName` = 'aura_area52_white_walker';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (978202, 'aura_area52_white_walker');

DELETE FROM `spell_script_names`
WHERE `spell_id` = 978302 AND `ScriptName` = 'aura_area52_slice_and_ice';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (978302, 'aura_area52_slice_and_ice');

DELETE FROM `spell_script_names`
WHERE `spell_id` = 979135 AND `ScriptName` = 'aura_area52_freezing_blood';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (979135, 'aura_area52_freezing_blood');

DELETE FROM `spell_script_names`
WHERE `spell_id` = 781046 AND `ScriptName` = 'aura_area52_fighters_sinister_strike';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (781046, 'aura_area52_fighters_sinister_strike');

DELETE FROM `spell_script_names`
WHERE `spell_id` = 978744 AND `ScriptName` = 'aura_area52_tides_of_battle';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (978744, 'aura_area52_tides_of_battle');

DELETE FROM `spell_script_names`
WHERE `spell_id` IN (965990, 965991, 965992, 965993, 965994, 965995, 965996, 965997, 965998)
  AND `ScriptName` = 'spell_area52_frostbrand_damage';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES
    (965990, 'spell_area52_frostbrand_damage'),
    (965991, 'spell_area52_frostbrand_damage'),
    (965992, 'spell_area52_frostbrand_damage'),
    (965993, 'spell_area52_frostbrand_damage'),
    (965994, 'spell_area52_frostbrand_damage'),
    (965995, 'spell_area52_frostbrand_damage'),
    (965996, 'spell_area52_frostbrand_damage'),
    (965997, 'spell_area52_frostbrand_damage'),
    (965998, 'spell_area52_frostbrand_damage');
