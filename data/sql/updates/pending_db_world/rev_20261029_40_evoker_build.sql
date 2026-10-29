-- Evoker build: the Mystic Enchants of the build that were not connected or did nothing.
-- Element-bound Arcane Missiles: Arcane Missiles damage (triggered missiles) rolls for Element-bound Power.
DELETE FROM `spell_proc` WHERE `SpellId` = 783324;
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(783324, 0, 3, 2097152, 0, 0, 65536, 1, 2, 0, 2, 0, 0, 100, 0, 0);

-- Frosty Frost Frostbolt's extra hit: 43.5% of spell power and 28% of attack power (tooltip).
DELETE FROM `spell_bonus_data` WHERE `entry` = 984624;
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(984624, 0.435, 0, 0.28, 0, 'Area 52 Frosty Frost Frostbolt');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_evoker', 'spell_area52_arcane_evoked', 'spell_area52_element_bound_missiles');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(953308, 'spell_area52_evoker'),
(953309, 'spell_area52_arcane_evoked'),
(783324, 'spell_area52_element_bound_missiles');
