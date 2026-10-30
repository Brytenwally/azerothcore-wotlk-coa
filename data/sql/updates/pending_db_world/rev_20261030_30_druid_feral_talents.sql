-- Savage Defense: melee critical strikes only; with Primal Gore's capstone, Lacerate's periodic criticals too
-- Feral Instinct rank 3: Swipe and Ravage damage takes 1 sec off Tiger's Fury and Enrage
DELETE FROM `spell_proc` WHERE `SpellId` IN (277401, 277402, 16949);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(277401, 0, 0, 0, 0, 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0),
(277402, 0, 0, 0, 0, 0, 0, 1, 2, 2, 0, 0, 0, 0, 0, 0),
(16949, 0, 7, 65536, 1048576, 1024, 16, 1, 2, 0, 0, 0, 0, 0, 0, 0);

-- Infected Wounds: Swipe too, at the tooltip's 50% chance
UPDATE `spell_proc` SET `SpellFamilyMask1` = 1049664, `SpellFamilyMask2` = 1024, `Chance` = 50 WHERE `SpellId` = -48483;

-- Improved Travel Form: one rank at a time, not all three stacked
DELETE FROM `spell_ranks` WHERE `first_spell_id` = 954198;
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(954198, 954198, 1),
(954198, 954199, 2),
(954198, 954200, 3);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_savage_defense_talent',
'aura_area52_feral_aggression', 'aura_area52_king_of_the_jungle', 'aura_area52_elder_hide',
'spell_area52_dreamstate_mana', 'aura_area52_earth_and_moon');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(277401, 'aura_area52_savage_defense_talent'),
(277402, 'aura_area52_savage_defense_talent'),
(-16858, 'aura_area52_feral_aggression'),
(48495, 'aura_area52_king_of_the_jungle'),
(16931, 'aura_area52_elder_hide'),
(833956, 'spell_area52_dreamstate_mana'),
(60433, 'aura_area52_earth_and_moon');
