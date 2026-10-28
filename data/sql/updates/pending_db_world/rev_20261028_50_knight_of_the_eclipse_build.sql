-- Knight of the Eclipse build: the Mystic Enchants of the build that were not connected or did nothing.
-- Knight of the Eclipse (Eclipse final rank 48525 required) replaces Sinister Strike with Eclipse Strike, Eviscerate
-- with Solar Strike and Starfall with Knightfall; Death Mark replaces Premeditation; Sunfire is taught beside
-- Moonfire. Rank chains come from SpellRank.dbc.
DELETE FROM `spell_ranks` WHERE `first_spell_id` IN (978617, 978618, 977832);
INSERT INTO `spell_ranks` (`first_spell_id`, `spell_id`, `rank`) VALUES
(978617, 978617, 1),
(978617, 978959, 2),
(978617, 978960, 3),
(978617, 978961, 4),
(978617, 978962, 5),
(978617, 978963, 6),
(978617, 978964, 7),
(978617, 978965, 8),
(978617, 978966, 9),
(978617, 978967, 10),
(978617, 978968, 11),
(978617, 978969, 12),
(978618, 978618, 1),
(978618, 978970, 2),
(978618, 978971, 3),
(978618, 978972, 4),
(978618, 978973, 5),
(978618, 978975, 6),
(978618, 978976, 7),
(978618, 978977, 8),
(978618, 978978, 9),
(978618, 978979, 10),
(978618, 978980, 11),
(978618, 978981, 12),
(977832, 977832, 1),
(977832, 977834, 2),
(977832, 977835, 3),
(977832, 977836, 4),
(977832, 977837, 5),
(977832, 977838, 6),
(977832, 977839, 7),
(977832, 977840, 8),
(977832, 977841, 9),
(977832, 977842, 10),
(977832, 977843, 11),
(977832, 977844, 12),
(977832, 977845, 13),
(977832, 977846, 14);

-- Solar Knight: next Wrath or Starsurge; Lunar Knight: next Starfire or Starsurge (both grant Astral Attunement).
-- Knight of the Eclipse loses a stack at each Wrath, Starfire or Starsurge cast. Infused Energy: direct damage
-- spells. Demonic Moonfire: Moonfire's periodic damage.
DELETE FROM `spell_proc` WHERE `SpellId` IN (978621, 978622, 979022, 979046, 782152);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(978621, 0, 7, 1, 0, 524288, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 1),
(978622, 0, 7, 4, 0, 524288, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 1),
(979022, 0, 7, 5, 0, 524288, 65536, 7, 1, 0, 0, 0, 0, 100, 0, 0),
(979046, 0, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(782152, 0, 7, 2, 0, 0, 262144, 1, 2, 0, 0, 0, 0, 100, 0, 0);

-- Solar Burn: 25.522% of spell power and attack power per tick (CoA tooltip); Knightfall's stars as Starfall's.
DELETE FROM `spell_bonus_data` WHERE `entry` IN (978623, 850288, 850294);
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(978623, 0, 0.25522, 0, 0.25522, 'Area 52 Solar Burn'),
(850288, 0.3, 0, 0, 0, 'Area 52 Knightfall Starfall'),
(850294, 0.13, 0, 0, 0, 'Area 52 Knightfall Starfall splash');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_eclipse_strike', 'spell_area52_solar_strike', 'spell_area52_knight_haste', 'spell_area52_eclipse_memory', 'spell_area52_sunfire', 'spell_area52_infused_energy', 'spell_area52_demonic_moonfire');
DELETE FROM `spell_script_names` WHERE `spell_id` IN (850286, 850294);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-978617, 'spell_area52_eclipse_strike'),
(-978618, 'spell_area52_solar_strike'),
(979022, 'spell_area52_knight_haste'),
(48517, 'spell_area52_eclipse_memory'),
(48518, 'spell_area52_eclipse_memory'),
(-977832, 'spell_area52_sunfire'),
(979046, 'spell_area52_infused_energy'),
(782152, 'spell_area52_demonic_moonfire'),
(850286, 'spell_dru_starfall_dummy'),
(850294, 'spell_dru_starfall_aoe');
