-- Compound Power grants the Hunter Compound Shot (826679, seven ranks by level, Hunter spellbook tab) in place of
-- the tab-less 984925. Its cast opens the 3 second recast window (826680), which turns the button into Combo Shot
-- (850515): a cone volley (850517) that bleeds enemies already in combat (Area52Marksman.inc).
DELETE FROM `spell_ranks` WHERE `first_spell_id` = 826679;
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(826679, 826679, 1),
(826679, 826681, 2),
(826679, 826682, 3),
(826679, 826683, 4),
(826679, 826684, 5),
(826679, 826685, 6),
(826679, 826686, 7);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_compound_shot', 'aura_area52_compound_shot_recast', 'spell_area52_combo_shot_volley');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-826679, 'spell_area52_compound_shot'),
(826680, 'aura_area52_compound_shot_recast'),
(850517, 'spell_area52_combo_shot_volley');
