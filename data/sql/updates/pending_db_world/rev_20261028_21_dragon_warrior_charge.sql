-- Dragon Warrior: Draconic Slam's proc spell 954260 already deals 200% weapon damage and cuts Dragon Roar's cooldown
-- by 5 seconds (Ascension effect 165), so the Dragon Fury cooldown script is dropped. Dragon Charge replaces
-- Intercept (AscensionMysticSpellGrants.h) and is scripted in Area52DragonWarrior.inc.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_area52_dragon_fury', 'aura_area52_dragon_charge', 'spell_area52_dragon_charge_impact');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(954256, 'aura_area52_dragon_charge'),
(954288, 'spell_area52_dragon_charge_impact');
