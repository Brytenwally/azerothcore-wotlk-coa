-- Omen of Clarity ranks 2 and 3 proc like rank 1 (they had no proc rule or script, so they never procced)
-- Empowered Touch rank 2: only Healing Touch and Flourish casts cool down Natural Alacrity and Tranquility
-- Nature's Focus capstone helper: being critically struck while casting a Nature heal
DELETE FROM `spell_proc` WHERE `SpellId` IN (816860, 816861, 33880, 717066);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(816860, 0, 0, 0, 0, 0, 81924, 3, 2, 0, 0, 0, 0, 0, 2000, 0),
(816861, 0, 0, 0, 0, 0, 81924, 3, 2, 0, 0, 0, 0, 0, 2000, 0),
(33880, 0, 7, 32, 0, 131072, 0, 7, 1, 0, 0, 0, 0, 0, 0, 0),
(717066, 0, 0, 0, 0, 0, 139944, 1, 2, 2, 0, 0, 0, 0, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (816860, 816861) AND `ScriptName` = 'spell_dru_omen_of_clarity';
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_capstone_grant', 'aura_area52_natures_focus_guard',
'spell_area52_intensity_mana', 'aura_area52_improved_tree_of_life', 'aura_area52_tree_form_talents');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(816860, 'spell_dru_omen_of_clarity'),
(816861, 'spell_dru_omen_of_clarity'),
(17066, 'aura_area52_capstone_grant'),
(17120, 'aura_area52_capstone_grant'),
(717066, 'aura_area52_natures_focus_guard'),
(817080, 'spell_area52_intensity_mana'),
(835358, 'spell_area52_intensity_mana'),
(835359, 'spell_area52_intensity_mana'),
(-48535, 'aura_area52_improved_tree_of_life'),
(33891, 'aura_area52_tree_form_talents'),
(86435, 'aura_area52_tree_form_talents');
