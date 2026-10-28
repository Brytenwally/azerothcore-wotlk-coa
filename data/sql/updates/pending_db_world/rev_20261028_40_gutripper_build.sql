-- Gutripper build (Blood and Guts): the Mystic Enchants of the build that were not connected or did nothing.
-- Blood and Guts grants Disembowel (12 ranks, Rogue tab) and its hidden Executing Disembowel passive (977908);
-- Piercing Cleaver grants Piercing Cleaver (8 ranks, Warrior tab). Their rank chains come from SpellRank.dbc.
DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (947470, 982966);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(947470, 947470, 1),
(947470, 947471, 2),
(947470, 947472, 3),
(947470, 947473, 4),
(947470, 947474, 5),
(947470, 947475, 6),
(947470, 947476, 7),
(947470, 947477, 8),
(947470, 947478, 9),
(947470, 947479, 10),
(947470, 947480, 11),
(947470, 947481, 12),
(982966, 982966, 1),
(982966, 983152, 2),
(982966, 983153, 3),
(982966, 983154, 4),
(982966, 983155, 5),
(982966, 983156, 6),
(982966, 983157, 7),
(982966, 983158, 8);

-- Mutilating Disembowel: each Mutilate weapon hit (family 8, mask1 0x2/0x4) on your Disemboweled target.
-- Executing Disembowel: Execute's damage spell (20647, mask0 0x20000000) on your Disemboweled target.
-- Judge, Jury, Executioner: Judgement damage (family 10, mask0 0x800000). Necrotic Execute: killing blows.
DELETE FROM `spell_proc` WHERE `SpellId` IN (977906, 977908, 84447, 782820);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(977906, 0, 8, 0, 6, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(977908, 0, 4, 536870912, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(84447, 0, 10, 8388608, 0, 0, 65552, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(782820, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 100, 0, 0);

-- Piercing Cleaver: ${$m1+($SP+$AP)*0.50} from its tooltip.
DELETE FROM `spell_bonus_data` WHERE `entry` = 982966;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(982966, 0.5, 0, 0.5, 0, 'Area 52 Piercing Cleaver');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_disembowel', 'spell_area52_blood_and_guts', 'spell_area52_executing_disembowel', 'spell_area52_executing_disembowel_burst', 'spell_area52_gutripper_execute', 'spell_area52_sudden_death_cooldown', 'spell_area52_necrotic_execute');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-947470, 'spell_area52_disembowel'),
(977906, 'spell_area52_blood_and_guts'),
(977908, 'spell_area52_executing_disembowel'),
(977909, 'spell_area52_executing_disembowel_burst'),
(20647, 'spell_area52_gutripper_execute'),
(-29723, 'spell_area52_sudden_death_cooldown'),
(782820, 'spell_area52_necrotic_execute');
