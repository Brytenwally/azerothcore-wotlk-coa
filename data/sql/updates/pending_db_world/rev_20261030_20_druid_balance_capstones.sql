-- Druid Balance capstones: Vengeful Soul on direct Arcane and Nature criticals, Starweaver's Starfall cooldown on
-- each Starsurge cast, Nature's Reach spreading Insect Swarm from non-periodic Nature damage (6 sec cooldown)
DELETE FROM `spell_proc` WHERE `SpellId` IN (16913, 984419, 816819);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(16913, 72, 0, 0, 0, 0, 65536, 1, 2, 2, 0, 0, 0, 0, 0, 0),
(984419, 0, 7, 0, 0, 524288, 65536, 7, 1, 0, 0, 0, 0, 0, 0, 0),
(816819, 8, 0, 0, 0, 0, 65536, 1, 2, 0, 0, 0, 0, 0, 6000, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_moonfury_dispel',
'aura_area52_insect_swarm_capstones', 'aura_area52_natures_splendor', 'aura_area52_vengeful_soul',
'aura_area52_natures_reach');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(-8921, 'aura_area52_moonfury_dispel'),
(-5570, 'aura_area52_insect_swarm_capstones'),
(857866, 'aura_area52_natures_splendor'),
(816913, 'aura_area52_vengeful_soul'),
(816819, 'aura_area52_natures_reach');
