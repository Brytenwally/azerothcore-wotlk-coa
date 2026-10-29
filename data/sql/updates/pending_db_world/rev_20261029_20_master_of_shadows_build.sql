-- Master of Shadows build: the Mystic Enchants of the build that were not connected or did nothing.
-- Master of Shadows: Sinister Strike, Sinister Flurry, Eviscerate, Garrote, Rupture and Crimson Tempest damage
-- (bleed ticks included) and Gloomblade's first hit trigger Dark Intent; Sinister Strike, Sinister Flurry and
-- Gloomblade criticals stack Apprentice of Shadows, which Eviscerate or Rupture spend at 3 stacks.
-- Pestilence: Sinister Strike hits. Shadow Strikes: damaging melee abilities, 5 second internal cooldown.
DELETE FROM `spell_proc` WHERE `SpellId` IN (980046, 81182, 979750);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(980046, 0, 0, 0, 0, 0, 262160, 7, 2, 0, 2, 2, 0, 100, 0, 0),
(81182, 0, 8, 2, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0),
(979750, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 50, 5000, 0);

-- Dark Intent: 11.5% of spell power and attack power (tooltip).
DELETE FROM `spell_bonus_data` WHERE `entry` IN (980050, 990248);
INSERT INTO `spell_bonus_data` (`entry`, `direct_bonus`, `dot_bonus`, `ap_bonus`, `ap_dot_bonus`, `comments`) VALUES
(980050, 0.115, 0, 0.115, 0, 'Area 52 Dark Intent'),
(990248, 0.115, 0, 0.115, 0, 'Area 52 Dark Intent');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_master_of_shadows', 'spell_area52_apprentice_of_shadows', 'spell_area52_pestilence', 'spell_area52_find_weakness');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(980046, 'spell_area52_master_of_shadows'),
(980048, 'spell_area52_apprentice_of_shadows'),
(81182, 'spell_area52_pestilence'),
(931234, 'spell_area52_find_weakness');
