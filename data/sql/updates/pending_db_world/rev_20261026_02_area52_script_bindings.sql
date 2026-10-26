-- Area 52 spell script bindings and the proc, bonus, rank and group rows their scripts check. The deployed
-- server has them and main lacked them, so the core left the Legendary and Epic Mystic Enchants with core
-- scripts (Hoplite, Mystic Knight, Battlemage, Blademaster, Bulwark, the legendary batches) unconnected.
-- Taken from CWO4PapaBear/azerothcore-wotlk-coa branches, in their date order. A delete by script name
-- alone is narrowed to the rows its insert adds where main binds that script to other spells too.

-- rev_20261005_00_area52_victory_rush.sql (fix/area52-victory-rush)
DELETE FROM `spell_script_names` WHERE `spell_id` = 32215 AND `ScriptName` = 'aura_wildcard_victorious_state';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(32215, 'aura_wildcard_victorious_state');

DELETE FROM `spell_script_names` WHERE `spell_id` = 34428 AND `ScriptName` = 'spell_area52_victory_rush_heal';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(34428, 'spell_area52_victory_rush_heal');

-- rev_20261006_01_area52_eldritch_knight.sql (fix/area52-eldritch-knight)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_eldritch_knight',
'spell_area52_eldritch_missing_mana', 'spell_area52_eldritch_condemnation');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(81116, 'aura_area52_eldritch_knight'),
(983708, 'spell_area52_eldritch_missing_mana'),
(983712, 'spell_area52_eldritch_missing_mana'),
(983703, 'spell_area52_eldritch_condemnation'),
(983704, 'spell_area52_eldritch_condemnation'),
(983705, 'spell_area52_eldritch_condemnation'),
(983706, 'spell_area52_eldritch_condemnation'),
(983707, 'spell_area52_eldritch_condemnation');

DELETE FROM `spell_proc` WHERE `SpellId` = 81116;

INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`,
`DisableEffectsMask`, `Chance`) VALUES
(81116, 20, 1, 2, 3, 4, 100);

-- rev_20261006_02_area52_mystic_knight_epics.sql (fix/area52-mystic-knight-epics)
DELETE FROM `spell_script_names` WHERE (`spell_id`, `ScriptName`) IN
    ((954083, 'aura_area52_deconstruction_power'), (92580, 'aura_area52_epic_equipment'), (926573, 'aura_area52_epic_equipment'), (954081, 'aura_area52_epic_equipment'), (954082, 'spell_area52_deconstruct'), (954443, 'spell_area52_deconstruct'), (954444, 'spell_area52_deconstruct'), (954445, 'spell_area52_deconstruct'), (954446, 'spell_area52_deconstruct'), (954447, 'spell_area52_deconstruct'), (954448, 'spell_area52_deconstruct'), (954449, 'spell_area52_deconstruct'), (954450, 'spell_area52_deconstruct'), (926575, 'spell_area52_emanate_damage'), (926577, 'spell_area52_emanate_damage'), (926579, 'spell_area52_emanate_damage'), (926581, 'spell_area52_emanate_damage'), (926583, 'spell_area52_emanate_damage'), (926585, 'spell_area52_emanate_damage'), (926587, 'spell_area52_emanate_damage'), (926589, 'spell_area52_emanate_damage'), (92583, 'spell_area52_flesh_hook'), (9654556, 'spell_area52_flesh_pull'));

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(92580, 'aura_area52_epic_equipment'),
(926573, 'aura_area52_epic_equipment'),
(954081, 'aura_area52_epic_equipment'),
(92583, 'spell_area52_flesh_hook'),
(9654556, 'spell_area52_flesh_pull'),
(954083, 'aura_area52_deconstruction_power'),
(926575, 'spell_area52_emanate_damage'),
(926577, 'spell_area52_emanate_damage'),
(926579, 'spell_area52_emanate_damage'),
(926581, 'spell_area52_emanate_damage'),
(926583, 'spell_area52_emanate_damage'),
(926585, 'spell_area52_emanate_damage'),
(926587, 'spell_area52_emanate_damage'),
(926589, 'spell_area52_emanate_damage'),
(954082, 'spell_area52_deconstruct'),
(954443, 'spell_area52_deconstruct'),
(954444, 'spell_area52_deconstruct'),
(954445, 'spell_area52_deconstruct'),
(954446, 'spell_area52_deconstruct'),
(954447, 'spell_area52_deconstruct'),
(954448, 'spell_area52_deconstruct'),
(954449, 'spell_area52_deconstruct'),
(954450, 'spell_area52_deconstruct');

-- rev_20261006_04_area52_hoplite_strike.sql (fix/area52-hoplite-shield-strike)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hoplite_shield_strike', 'aura_area52_hoplite_flurry', 'spell_area52_hoplite_flurry_damage');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978533, 'spell_area52_hoplite_shield_strike'),
(300158, 'spell_area52_hoplite_shield_strike'),
(901200, 'aura_area52_hoplite_flurry'),
(901201, 'spell_area52_hoplite_flurry_damage');

DELETE FROM `spell_script_names` WHERE `spell_id` = 978217 AND `ScriptName` = 'aura_area52_hoplite_equipment';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (978217, 'aura_area52_hoplite_equipment');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_hoplite_requirements';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978533, 'spell_area52_hoplite_requirements'),
(300158, 'spell_area52_hoplite_requirements'),
(978569, 'spell_area52_hoplite_requirements'),
(982250, 'spell_area52_hoplite_requirements'),
(901200, 'spell_area52_hoplite_requirements');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hoplite_thrust', 'spell_area52_hoplite_bloodrage', 'aura_area52_hoplite_phalanx_damage', 'spell_area52_hoplite_javelin', 'aura_area52_hoplite_impaled', 'spell_area52_hoplite_colossus', 'spell_area52_hoplite_flurry');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(978569, 'spell_area52_hoplite_thrust'),
(2687, 'spell_area52_hoplite_bloodrage'),
(978539, 'aura_area52_hoplite_phalanx_damage'),
(978537, 'aura_area52_hoplite_phalanx_damage'),
(982250, 'spell_area52_hoplite_javelin'),
(982251, 'aura_area52_hoplite_impaled'),
(-86361, 'spell_area52_hoplite_colossus'),
(901200, 'spell_area52_hoplite_flurry');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_hoplite_incompatible';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(812295, 'aura_area52_hoplite_incompatible'),
(812676, 'aura_area52_hoplite_incompatible'),
(812677, 'aura_area52_hoplite_incompatible'),
(812300, 'aura_area52_hoplite_incompatible'),
(812959, 'aura_area52_hoplite_incompatible'),
(812960, 'aura_area52_hoplite_incompatible'),
(829623, 'aura_area52_hoplite_incompatible'),
(829634, 'aura_area52_hoplite_incompatible'),
(1584277, 'aura_area52_hoplite_incompatible'),
(954058, 'aura_area52_hoplite_incompatible'),
(277791, 'aura_area52_hoplite_incompatible');

DELETE FROM `spell_script_names` WHERE `spell_id` = 277595 AND `ScriptName` = 'spell_area52_hoplite_incompatible';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (277595, 'spell_area52_hoplite_incompatible');

-- rev_20261008_09_area52_advancement_procs.sql (fix/area52-advancement-proc-batch)
DELETE FROM `spell_proc` WHERE `SpellId` IN (12753,12308,12810,12811,14142,14769,816968,31682,31683,31684,33225);

INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(12753,0,0,0,0,0,40,0,2,16,0,0,0,0,0,0),
(12308,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(12810,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(12811,0,0,0,0,0,69648,1,2,0,0,0,0,0,0,0),
(14142,0,0,0,0,0,262144,1,2,0,0,0,0,100,0,0),
(14769,0,0,0,0,0,17408,4,2,0,0,0,0,100,0,0),
(816968,0,0,0,0,0,16,1,2,0,0,0,0,100,0,0),
(31682,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(31683,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(31684,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(33225,0,0,0,0,0,262144,1,2,0,0,0,0,30,0,0);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_advancement_proc' AND `spell_id` IN (12753,12308,12810,12811,14142,14769,816968,31682,31683,31684,33225);

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(12753,'spell_area52_advancement_proc'),
(12308,'spell_area52_advancement_proc'),
(12810,'spell_area52_advancement_proc'),
(12811,'spell_area52_advancement_proc'),
(14142,'spell_area52_advancement_proc'),
(14769,'spell_area52_advancement_proc'),
(816968,'spell_area52_advancement_proc'),
(31682,'spell_area52_advancement_proc'),
(31683,'spell_area52_advancement_proc'),
(31684,'spell_area52_advancement_proc'),
(33225,'spell_area52_advancement_proc');

-- rev_20261008_10_area52_followup_procs.sql (fix/area52-followup-proc-batch)
DELETE FROM `spell_proc` WHERE `SpellId` IN (16288,63506,17958,34493,12318,12857,16219,863372);

INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(16288,0,0,0,0,0,17408,0,1,0,0,0,0,100,20000,0),
(63506,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(17958,0,0,0,0,0,69632,0,1,0,0,4,0,100,0,0),
(34493,0,0,0,0,0,262144,1,2,0,0,0,0,100,1000,0),
(12318,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(12857,0,0,0,0,0,17408,0,1,0,0,0,0,100,0,0),
(16219,0,0,0,0,0,17408,2,2,2,0,0,0,100,0,0),
(863372,0,0,0,0,0,65536,1,2,0,0,0,0,100,0,0);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_followup_proc' AND `spell_id` IN (16288,63506,17958,34493,12318,12857,16219,863372);

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(16288,'spell_area52_followup_proc'),
(63506,'spell_area52_followup_proc'),
(17958,'spell_area52_followup_proc'),
(34493,'spell_area52_followup_proc'),
(12318,'spell_area52_followup_proc'),
(12857,'spell_area52_followup_proc'),
(16219,'spell_area52_followup_proc'),
(863372,'spell_area52_followup_proc');

-- rev_20261008_11_area52_stateful_procs.sql (fix/area52-followup-proc-batch)
DELETE FROM `spell_proc` WHERE `SpellId` IN (20361,20362,811100,20042,20045,851561,44400,44402,44403,760191,19298);

INSERT INTO `spell_proc` (`SpellId`,`SchoolMask`,`SpellFamilyName`,`SpellFamilyMask0`,`SpellFamilyMask1`,`SpellFamilyMask2`,`ProcFlags`,`SpellTypeMask`,`SpellPhaseMask`,`HitMask`,`AttributesMask`,`DisableEffectsMask`,`ProcsPerMinute`,`Chance`,`Cooldown`,`Charges`) VALUES
(20361,0,0,0,0,0,69632,0,1,0,0,0,0,100,0,0),
(20362,0,0,0,0,0,87040,0,4,0,8,0,0,100,0,1),
(811100,0,0,0,0,0,65536,0,4,0,8,0,0,100,0,1),
(20042,0,0,0,0,0,17408,0,2,0,0,2,0,100,0,0),
(20045,0,0,0,0,0,17408,0,2,0,0,2,0,100,0,0),
(851561,0,0,0,0,0,17408,2,2,0,2,0,0,100,5000,0),
(44400,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(44402,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(44403,0,0,0,0,0,65536,0,1,0,0,0,0,100,0,0),
(760191,0,0,0,0,0,17408,2,2,0,0,0,0,100,0,0),
(19298,0,0,0,0,0,16,1,2,2,0,0,0,100,4000,0);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_followup_proc' AND `spell_id` IN (20361,811100,20042,20045,851561,44400,44402,44403,760191,19298);

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(20361,'spell_area52_followup_proc'),
(811100,'spell_area52_followup_proc'),
(20042,'spell_area52_followup_proc'),
(20045,'spell_area52_followup_proc'),
(851561,'spell_area52_followup_proc'),
(44400,'spell_area52_followup_proc'),
(44402,'spell_area52_followup_proc'),
(44403,'spell_area52_followup_proc'),
(760191,'spell_area52_followup_proc'),
(19298,'spell_area52_followup_proc');

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_flame_throwing' AND `spell_id`=812352;

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (812352,'spell_area52_flame_throwing');

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_area52_shared_might' AND `spell_id`=920043;

INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES (920043,'spell_area52_shared_might');

-- rev_20261009_02_area52_legendary_procs.sql (fix/area52-legendary-batch7)
DELETE FROM `spell_proc` WHERE `SpellId` IN (84508, 84512, 965500, 965501, 965502);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84508, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(84512, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(965500, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 35, 0, 0),
(965501, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 3000, 0),
(965502, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 40, 3000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_legendary_proc', 'spell_area52_purification_hit', 'aura_area52_purification_ground', 'spell_area52_zealot_hit');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84508, 'spell_area52_legendary_proc'),
(84512, 'spell_area52_legendary_proc'),
(965500, 'spell_area52_legendary_proc'),
(965501, 'spell_area52_legendary_proc'),
(965502, 'spell_area52_legendary_proc'),
(84509, 'spell_area52_purification_hit'),
(84759, 'aura_area52_purification_ground'),
(965508, 'spell_area52_zealot_hit');

DELETE FROM `spell_bonus_data` WHERE `entry` = 84760;

INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(84760, 0.2, 0, 0.135, 0, 'Area 52 - Amal\'thazad\'s Curse');

DELETE FROM `spell_bonus_data` WHERE `entry` = 84509;

INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`)
SELECT 84509, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, 'Area 52 - Purification Exorcism'
FROM `spell_bonus_data` WHERE `entry` = 879;

DELETE FROM `spell_bonus_data` WHERE `entry` = 84759;

INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`)
SELECT 84759, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, 'Area 52 - Purification Consecration'
FROM `spell_bonus_data` WHERE `entry` = 26573;

-- rev_20261009_03_area52_battlemage.sql (fix/area52-battlemage)
DELETE FROM `spell_proc` WHERE `SpellId` = 81258;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(81258, 0, 4, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 4000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_battlemage', 'spell_area52_battlemage_damage');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
SELECT 81258, 'spell_area52_battlemage'
UNION SELECT `spell_id`, 'spell_area52_battlemage_damage' FROM `spell_ranks` WHERE `first_spell_id` IN (44425, 2136, 30455)
UNION SELECT 44425, 'spell_area52_battlemage_damage'
UNION SELECT 2136, 'spell_area52_battlemage_damage'
UNION SELECT 30455, 'spell_area52_battlemage_damage';

-- rev_20261009_04_area52_legendary_batch.sql (fix/area52-legendary-batch10)
DELETE FROM `spell_proc` WHERE `SpellId` IN (84510, 965503, 965504, 965505, 81119, 81122, 81479, 965315, 84748, 983908);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84510, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 1, 0, 33, 4000, 0),
(965503, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 3, 0, 33, 6000, 0),
(965504, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 12000, 0),
(965505, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 33, 0, 0),
(81119, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(81122, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(81479, 0, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(965315, 0, 0, 0, 0, 0, 327952, 1, 2, 0, 2, 5, 0, 100, 0, 0),
(84748, 0, 0, 0, 0, 0, 87312, 7, 1, 0, 0, 3, 0, 100, 0, 1),
(983908, 0, 0, 0, 0, 0, 87312, 7, 1, 0, 0, 3, 0, 100, 0, 2);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_legendary_batch', 'spell_area52_slinger_consumption', 'spell_area52_bone_chill_vulnerability');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(84510, 'spell_area52_legendary_batch'),
(965503, 'spell_area52_legendary_batch'),
(965504, 'spell_area52_legendary_batch'),
(965505, 'spell_area52_legendary_batch'),
(81119, 'spell_area52_legendary_batch'),
(81122, 'spell_area52_legendary_batch'),
(81479, 'spell_area52_legendary_batch'),
(965315, 'spell_area52_legendary_batch'),
(84748, 'spell_area52_slinger_consumption'),
(983908, 'spell_area52_slinger_consumption'),
(965706, 'spell_area52_bone_chill_vulnerability');

DELETE FROM `spell_proc` WHERE `SpellId` = 99634;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(99634, 0, 0, 0, 0, 0, 20, 1, 2, 0, 0, 6, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_corrupted_bear', 'spell_area52_corrupted_bear_form');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(99634, 'spell_area52_corrupted_bear'),
(5487, 'spell_area52_corrupted_bear_form'),
(9634, 'spell_area52_corrupted_bear_form');

DELETE FROM `spell_bonus_data` WHERE `entry` IN (84511, 965509, 965510, 965316);

INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(84511, 0.25, 0, 0.18, 0, 'Area52 Unbounded Spell Slinger'),
(965509, 0.7, 0, 0.5, 0, 'Area52 Spell Slinger'),
(965510, 0.3656, 0, 0.2376, 0, 'Area52 Frost Lich root tooltip contract'),
(965316, 0, 0.015, 0, 0.01035, 'Area52 Wild Felfire per tick');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_felfire_equipment', 'spell_area52_pure_chaos_bolt');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(965315, 'spell_area52_felfire_equipment'),
(944811, 'spell_area52_pure_chaos_bolt'),
(944812, 'spell_area52_pure_chaos_bolt'),
(944813, 'spell_area52_pure_chaos_bolt'),
(944814, 'spell_area52_pure_chaos_bolt');

DELETE FROM `spell_ranks` WHERE `spell_id` IN (944811, 944812, 944813, 944814);

INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(944811, 944811, 1),
(944811, 944812, 2),
(944811, 944813, 3),
(944811, 944814, 4);

DELETE FROM `spell_proc` WHERE `SpellId` = 954046;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(954046, 24, 0, 0, 0, 0, 65808, 1, 2, 0, 0, 2, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_overloaded_mind', 'spell_area52_froststorm_echo');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(954046, 'spell_area52_overloaded_mind'),
(954048, 'spell_area52_froststorm_echo');

DELETE FROM `spell_bonus_data` WHERE `entry` IN (954048, 954049);

INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(954048, 0.0667, 0, 0.0434, 0, 'Area52 Overloaded Froststorm'),
(954049, 0.11, 0, 0.71, 0, 'Area52 Astral Tempest');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_felfire_emberstorm';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(411119, 'spell_area52_felfire_emberstorm');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_corrupted_bear_instant';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(17941, 'spell_area52_corrupted_bear_instant'),
(34936, 'spell_area52_corrupted_bear_instant');

-- rev_20261009_05_area52_legendary_dependencies.sql (fix/area52-legendary-dependency-repairs)
DELETE FROM `spell_proc` WHERE `SpellId` IN (-701463, -48393, -48392, -48389, -44545, -44543, -44448, -44446, -44445, -18095, -18094, -17941, -12575, -12574, -12536, -11213, 11213, 12536, 12574, 12575, 17941, 18094, 18095, 44445, 44446, 44448, 44543, 44545, 48389, 48392, 48393, 701463);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(11213, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 5, 0, 0),
(12574, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 10, 0, 0),
(12575, 0, 0, 0, 0, 0, 87376, 1, 2, 0, 0, 0, 0, 15, 0, 0),
(18094, 0, 0, 0, 0, 0, 327680, 1, 2, 0, 0, 0, 0, 4, 5000, 0),
(18095, 0, 0, 0, 0, 0, 327680, 1, 2, 0, 0, 0, 0, 8, 5000, 0),
(44445, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44446, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44448, 0, 0, 0, 0, 0, 65536, 1, 2, 3, 0, 0, 0, 100, 0, 0),
(44543, 16, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 6, 0, 7, 0, 0),
(44545, 16, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 6, 0, 15, 0, 0),
(48389, 0, 0, 0, 0, 0, 139944, 1, 2, 0, 0, 0, 0, 8, 0, 0),
(48392, 0, 0, 0, 0, 0, 131752, 1, 2, 0, 0, 0, 0, 16, 0, 0),
(48393, 0, 0, 0, 0, 0, 131752, 1, 2, 0, 0, 0, 0, 24, 0, 0),
(12536, 0, 0, 0, 0, 0, 87312, 0, 1, 0, 12, 0, 0, 100, 0, 1),
(17941, 0, 5, 1, 16, 0, 65536, 1, 1, 0, 8, 0, 0, 100, 0, 1),
(701463, 0, 0, 0, 0, 0, 40, 1, 2, 1027, 0, 5, 0, 30, 2000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_dependency_proc';

DELETE FROM `spell_script_names` WHERE `spell_id` IN (-18094, 18094, 18095) AND `ScriptName` = 'spell_warl_nightfall';

DELETE FROM `spell_script_names` WHERE `spell_id` IN (-44445, 44445, 44446, 44448) AND `ScriptName` = 'spell_mage_hot_streak';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(18094, 'spell_area52_dependency_proc'),
(18095, 'spell_area52_dependency_proc'),
(44445, 'spell_area52_dependency_proc'),
(44446, 'spell_area52_dependency_proc'),
(44448, 'spell_area52_dependency_proc'),
(44543, 'spell_area52_dependency_proc'),
(44545, 'spell_area52_dependency_proc'),
(48389, 'spell_area52_dependency_proc'),
(48392, 'spell_area52_dependency_proc'),
(48393, 'spell_area52_dependency_proc');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_deep_freeze';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (44572, 'spell_area52_deep_freeze');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_mana_barrier', 'spell_area52_siegebreaker', 'aura_area52_siegebreaker_collision');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (701463, 'spell_area52_mana_barrier'), (-954876, 'spell_area52_siegebreaker'), (954877, 'aura_area52_siegebreaker_collision');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_missing_mana_surge';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (277432, 'aura_area52_missing_mana_surge');

DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (33876, 33878) OR `spell_id` IN (33876, 33878, 33884, 33885, 33982, 33983, 33986, 33987, 48563, 48564, 48565, 48566, 302027, 302028, 302029, 302030, 302031, 302032, 302033, 302034);

INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(33876, 33876, 1),
(33876, 302027, 2),
(33876, 302028, 3),
(33876, 302029, 4),
(33876, 302030, 5),
(33876, 33982, 6),
(33876, 33983, 7),
(33876, 48565, 8),
(33876, 48566, 9),
(33878, 33878, 1),
(33878, 302031, 2),
(33878, 302032, 3),
(33878, 302033, 4),
(33878, 302034, 5),
(33878, 33986, 6),
(33878, 33987, 7),
(33878, 48563, 8),
(33878, 48564, 9);

-- rev_20261010_00_area52_hoplite_support.sql (feat/area52-hoplite-build-audit)
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-12317, 12317, 13045, 13046, 13047, 13048) AND `ScriptName` = 'spell_gen_proc_from_direct_damage';

DELETE FROM `spell_script_names` WHERE `spell_id` IN (-51698, 51698, 51700, 51701) AND `ScriptName` = 'spell_rog_honor_among_thieves';

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_hoplite_support_proc';

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_sweeping_swings';

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_shattering_throw';

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_sweeping_swings_damage';

DELETE FROM `spell_script_names` WHERE `spell_id` = 64380 AND `ScriptName` = 'spell_warr_shattering_throw';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(12317, 'spell_area52_hoplite_support_proc'),
(13045, 'spell_area52_hoplite_support_proc'),
(13046, 'spell_area52_hoplite_support_proc'),
(13047, 'spell_area52_hoplite_support_proc'),
(13048, 'spell_area52_hoplite_support_proc'),
(51701, 'spell_area52_hoplite_support_proc'),
(84445, 'spell_area52_hoplite_support_proc'),
(812297, 'spell_area52_hoplite_support_proc'),
(912326, 'spell_area52_hoplite_support_proc'),
(912326, 'aura_area52_sweeping_swings'),
(64380, 'spell_area52_shattering_throw'),
(12328, 'aura_area52_sweeping_swings'),
(997604, 'spell_area52_sweeping_swings_damage');

DELETE FROM `spell_proc` WHERE `SpellId` IN (-12317, 12317, 13045, 13046, 13047, 13048, -51698, 51698, 51700, 51701, -84445, 84445);

DELETE FROM `spell_proc` WHERE `SpellId` IN (-812297, 812297);

DELETE FROM `spell_proc` WHERE `SpellId` IN (-912326, 912326);

DELETE FROM `spell_proc` WHERE `SpellId` IN (-12834, 12834, 12849, 12867);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(12317, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13045, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13046, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13047, 0, 0, 0, 0, 0, 17408, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(13048, 0, 0, 0, 0, 0, 157352, 7, 2, 0, 0, 0, 0, 100, 5000, 0),
(51701, 0, 0, 0, 0, 0, 69904, 1, 2, 0, 0, 6, 0, 100, 3000, 0),
(84445, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 0, 0, 100, 20000, 0),
(812297, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 3, 0, 100, 0, 0),
(912326, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 5, 0, 100, 1000, 0),
(12834, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0),
(12849, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0),
(12867, 0, 0, 0, 0, 0, 20, 1, 2, 2, 0, 0, 0, 100, 0, 0);

-- rev_20261010_01_area52_hoplite_remaining.sql (feat/area52-hoplite-build-audit)
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_hunger_for_blood', 'aura_area52_wind_slam', 'spell_area52_wind_slam_damage', 'aura_area52_draconic_sinister');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(51662, 'spell_area52_hunger_for_blood'),
(979187, 'aura_area52_wind_slam'),
(979188, 'aura_area52_wind_slam'),
(979189, 'spell_area52_wind_slam_damage'),
(782246, 'aura_area52_draconic_sinister');

DELETE FROM `spell_proc` WHERE `SpellId` IN (979187, 979188, 782246);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(979187, 0, 11, 8388608, 0, 0, 16, 1, 2, 0, 2, 2, 0, 100, 0, 0),
(979188, 0, 0, 0, 0, 0, 69904, 1, 2, 0, 2, 0, 0, 100, 0, 0),
(782246, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_group` WHERE `id` = 1059 AND `spell_id` = 979189;

INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(1059, 979189);

-- rev_20261010_90_area52_blademaster.sql (fix/area52-blademaster)
DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (20243, 53385) OR `spell_id` IN (20243, 302035, 302036, 302037, 302038, 30016, 30022, 47497, 47498, 53385, 54762, 54763, 54764, 54765, 54766, 54767);

INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(20243, 20243, 1),
(20243, 302035, 2),
(20243, 302036, 3),
(20243, 302037, 4),
(20243, 302038, 5),
(20243, 30016, 6),
(20243, 30022, 7),
(20243, 47497, 8),
(20243, 47498, 9),
(53385, 53385, 1),
(53385, 54762, 2),
(53385, 54763, 3),
(53385, 54764, 4),
(53385, 54765, 5),
(53385, 54766, 6),
(53385, 54767, 7);

DELETE FROM `spell_proc` WHERE `SpellId` = 965862;

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(965862, 0, 0, 0, 0, 0, 56, 1, 2, 34, 0, 6, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE (`spell_id`, `ScriptName`) IN
    ((965862, 'aura_area52_blademaster'), (-20243, 'spell_area52_blademaster_empower'), (14251, 'spell_area52_blademaster_empower'), (-53385, 'spell_area52_blademaster_empower'), (12295, 'aura_area52_tactical_mastery'), (12676, 'aura_area52_tactical_mastery'), (12677, 'aura_area52_tactical_mastery'), (20231, 'aura_area52_tactical_stagger'), (20232, 'aura_area52_tactical_debt'));

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(965862, 'aura_area52_blademaster'),
(-20243, 'spell_area52_blademaster_empower'),
(14251, 'spell_area52_blademaster_empower'),
(-53385, 'spell_area52_blademaster_empower'),
(12295, 'aura_area52_tactical_mastery'),
(12676, 'aura_area52_tactical_mastery'),
(12677, 'aura_area52_tactical_mastery'),
(20231, 'aura_area52_tactical_stagger'),
(20232, 'aura_area52_tactical_debt');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (53385, -53385, 54762, 54763, 54764, 54765, 54766, 54767) AND `ScriptName` = 'spell_pal_divine_storm';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-53385, 'spell_pal_divine_storm');

-- rev_20261010_91_area52_spartan_kick.sql (fix/area52-spartan-kick)
DELETE FROM `spell_script_names` WHERE `spell_id` = 84445 AND `ScriptName` IN ('spell_area52_hoplite_support_proc', 'aura_area52_spartan_kick');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (84445, 'aura_area52_spartan_kick');

DELETE FROM `spell_proc` WHERE `SpellId` IN (-84445, 84445);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(84445, 0, 0, 0, 0, 0, 69648, 7, 2, 0, 0, 0, 0, 100, 20000, 0);

-- rev_20261012_10_area52_bulwark.sql (fix/area52-bulwark-build)
DELETE FROM `spell_script_names` WHERE `spell_id` IN (-59088, 59088, 59089, 859089) AND `ScriptName` = 'spell_warr_improved_spell_reflection';

DELETE FROM `spell_proc` WHERE `SpellId` IN (-59088, 59088, 59089, 859089, 977828, 978724, 922565, 86378);

INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(977828, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(978724, 0, 0, 0, 0, 0, 680, 1, 0, 8256, 0, 0, 0, 100, 1000, 0),
(922565, 0, 0, 0, 0, 0, 139944, 1, 0, 9283, 0, 6, 0, 100, 0, 0),
(86378, 0, 0, 0, 0, 0, 139944, 1, 0, 9283, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_proc` WHERE `SpellId` IN (-20127, 20127, 20130, 20135, 20100);

INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `DisableEffectsMask`, `Chance`, `Cooldown`) VALUES
(20127, 680, 1, 0, 8256, 2, 10, 0),
(20130, 680, 1, 0, 8256, 2, 10, 0),
(20135, 680, 1, 0, 8256, 2, 10, 0),
(20100, 680, 1, 0, 8256, 5, 100, 1000);

UPDATE `spell_bonus_data` SET `ap_bonus` = 0.69 WHERE `entry` = 1495;

DELETE FROM `spell_proc` WHERE `SpellId` = 20095;

INSERT INTO `spell_proc` (`SpellId`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `DisableEffectsMask`, `Chance`, `Charges`) VALUES
(20095, 680, 1, 0, 8256, 0, 100, 2);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_shield_tools', 'aura_area52_bulwark_cleanup', 'aura_area52_wild_aggression', 'aura_area52_bulwark_energy', 'spell_area52_bulwark_channel', 'aura_area52_bulwark_drain', 'spell_area52_shockwave', 'spell_area52_shockwave_damage', 'spell_area52_bulwark_damage', 'aura_area52_combat_readiness');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(871, 'spell_area52_shield_tools'),
(2565, 'spell_area52_shield_tools'),
(23920, 'spell_area52_shield_tools'),
(-23922, 'spell_area52_shield_tools'),
(859089, 'aura_area52_bulwark_cleanup'),
(986125, 'aura_area52_bulwark_cleanup'),
(977828, 'aura_area52_bulwark_cleanup'),
(901144, 'aura_area52_bulwark_cleanup'),
(978724, 'aura_area52_bulwark_cleanup'),
(977828, 'aura_area52_wild_aggression'),
(978724, 'aura_area52_bulwark_energy'),
(922565, 'spell_area52_bulwark_channel'),
(923923, 'aura_area52_bulwark_drain'),
(46968, 'spell_area52_shockwave'),
(846968, 'spell_area52_shockwave_damage'),
(846969, 'spell_area52_shockwave_damage'),
(923921, 'spell_area52_bulwark_damage'),
(923922, 'spell_area52_bulwark_damage'),
(86378, 'aura_area52_combat_readiness'),
(886378, 'aura_area52_combat_readiness');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_intervene_taunt', 'spell_area52_primal_precision');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(3411, 'spell_area52_intervene_taunt'),
(-1079, 'spell_area52_primal_precision'),
(-22568, 'spell_area52_primal_precision'),
(-22570, 'spell_area52_primal_precision'),
(52610, 'spell_area52_primal_precision');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_tank_stance';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(71, 'aura_area52_tank_stance'),
(48263, 'aura_area52_tank_stance'),
(25780, 'aura_area52_tank_stance'),
(701463, 'aura_area52_tank_stance'),
(5487, 'aura_area52_tank_stance'),
(9634, 'aura_area52_tank_stance');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'aura_area52_avatar_movement';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (86380, 'aura_area52_avatar_movement');
