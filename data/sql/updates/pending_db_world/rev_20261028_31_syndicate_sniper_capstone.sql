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
