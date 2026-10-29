-- Tree of Wrath build: the Mystic Enchants of the build that were not connected or did nothing.
-- Tree of Wrath replaces Tree of Life with the Tree of Wrath form and Chain Heal with Living Lightning, whose rank
-- chain comes from SpellRank.dbc. Living Lightning keeps Chain Heal's Riptide bonus.
DELETE FROM `spell_ranks` WHERE `first_spell_id` = 86441;
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(86441, 86441, 1),
(86441, 86442, 2),
(86441, 86443, 3),
(86441, 86444, 4),
(86441, 86445, 5),
(86441, 86446, 6),
(86441, 86447, 7),
(86441, 86448, 8);

-- Tree of Wrath: Lightning Bolt damage and direct heals (Lightning Rod, Living Lightning cooldown).
-- Nature's Flow: critical direct Shaman heals, 1 second internal cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` IN (86436, 983470);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(86436, 0, 0, 0, 0, 0, 81920, 3, 2, 0, 0, 0, 0, 100, 0, 0),
(983470, 0, 11, 448, 0, 16, 16384, 2, 2, 2, 0, 0, 0, 100, 1000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_tree_of_wrath_form', 'spell_area52_tree_of_wrath', 'spell_area52_living_lightning_rod', 'spell_area52_transcendental_embrace', 'spell_area52_transcendental_jump');
DELETE FROM `spell_script_names` WHERE `spell_id` = -86441 AND `ScriptName` = 'spell_sha_chain_heal';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(86435, 'spell_area52_tree_of_wrath_form'),
(86436, 'spell_area52_tree_of_wrath'),
(86449, 'spell_area52_living_lightning_rod'),
(86450, 'spell_area52_living_lightning_rod'),
(86451, 'spell_area52_living_lightning_rod'),
(86453, 'spell_area52_living_lightning_rod'),
(86455, 'spell_area52_living_lightning_rod'),
(86456, 'spell_area52_living_lightning_rod'),
(86457, 'spell_area52_living_lightning_rod'),
(86458, 'spell_area52_living_lightning_rod'),
(911127, 'spell_area52_transcendental_embrace'),
(411127, 'spell_area52_transcendental_jump'),
(-86441, 'spell_sha_chain_heal');
