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
