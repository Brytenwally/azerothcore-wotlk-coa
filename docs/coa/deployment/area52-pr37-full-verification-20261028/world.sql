-- Marksman build (Compound Power): the four Mystic Enchants of the build that were not connected.
-- Compound Power (979740) and Sniper (81166) get their scripts (Area52Marksman.inc); Munitions Expert, Syndicate
-- Sniper Shot and Compound Power's Compound Shot are spell grants (AscensionMysticSpellGrants.h).
-- Compound Power procs on every damaging spell; its script keeps Steady, Arcane, Multi-, Chimera and Compound Shot.
DELETE FROM `spell_proc` WHERE `SpellId` = 979740;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(979740, 0, 0, 0, 0, 0, 69888, 1, 2, 0, 0, 2, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_compound_power', 'spell_area52_compound_power_slice_and_dice', 'spell_area52_sniper');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(979740, 'spell_area52_compound_power'),
(-5171, 'spell_area52_compound_power_slice_and_dice'),
(81166, 'spell_area52_sniper');

-- Heavy Munitions and High Velocity Munitions replace each other.
DELETE FROM `spell_group` WHERE `id` = 2000979;
INSERT INTO `spell_group` (`id`, `spell_id`) VALUES
(2000979, 978717),
(2000979, 978718);
DELETE FROM `spell_group_stack_rules` WHERE `group_id` = 2000979;
INSERT INTO `spell_group_stack_rules` (`group_id`, `stack_rule`, `description`) VALUES
(2000979, 1, 'Munitions Expert: Heavy Munitions or High Velocity Munitions');

-- Dragon Warrior build: the four Mystic Enchants of the build that were not connected.
-- Slam's damage spell is cast with proc events disabled (spell_warr_slam), so the Slam procs read the Slam cast
-- itself (family mask 0x200000, any spell type). Skull Crusher and Battering Ram only lacked their proc rows;
-- Skull Crusher's 3 second limit is its tooltip's. Dragon Warrior: Bloodthirst and Execute grant Draconic Slam,
-- which the next Slam consumes (one charge) for a Dragon Fury stack; Dragon Roar replaces Death Wish
-- (AscensionMysticSpellGrants.h). Unstoppable Force is scripted in Area52DragonWarrior.inc.
DELETE FROM `spell_proc` WHERE `SpellId` IN (81101, 964332, 954252, 954261);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(81101, 0, 4, 2097152, 0, 0, 16, 7, 2, 0, 0, 0, 0, 100, 0, 0),
(964332, 0, 4, 2097152, 0, 0, 16, 7, 2, 0, 0, 0, 0, 100, 0, 0),
(954252, 0, 4, 536870912, 1024, 0, 16, 7, 2, 0, 0, 0, 0, 100, 0, 0),
(954261, 0, 4, 2097152, 0, 0, 16, 7, 2, 0, 0, 0, 0, 100, 0, 1);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_dragon_fury', 'spell_area52_dragon_roar_stun');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(954253, 'spell_area52_dragon_fury'),
(954251, 'spell_area52_dragon_roar_stun');

-- Dragon Warrior: Draconic Slam's proc spell 954260 already deals 200% weapon damage and cuts Dragon Roar's cooldown
-- by 5 seconds (Ascension effect 165), so the Dragon Fury cooldown script is dropped. Dragon Charge replaces
-- Intercept (AscensionMysticSpellGrants.h) and is scripted in Area52DragonWarrior.inc.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_dragon_fury', 'aura_area52_dragon_charge', 'spell_area52_dragon_charge_impact');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(954256, 'aura_area52_dragon_charge'),
(954288, 'spell_area52_dragon_charge_impact');

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

-- Syndicate Sniper Shot: with the enchant on, Sniper Training (53302-53304) keeps the matching Syndicate Sniper
-- Training stack (983867-983869) in step with its own buff. At 4 stacks of the third rank, an Aimed Shot or Compound
-- Shot hit on a creature grants Hone In (983887, 983888: the next Syndicate Sniper Shot without stealth), every
-- 8 seconds at most; the shot spends it (Area52Marksman.inc).
DELETE FROM `spell_proc` WHERE `SpellId` = 983869;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(983869, 0, 9, 131072, 0, 0, 256, 1, 2, 0, 0, 0, 0, 100, 8000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_syndicate_sniper_training', 'spell_area52_syndicate_capstone', 'spell_area52_syndicate_sniper_shot');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-53302, 'spell_area52_syndicate_sniper_training'),
(983869, 'spell_area52_syndicate_capstone'),
(982891, 'spell_area52_syndicate_sniper_shot');

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_area52_skull_crusher_cooldown';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(179, 'spell_area52_skull_crusher_cooldown');

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
