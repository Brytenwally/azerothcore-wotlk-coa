DELETE FROM `spell_script_names`
WHERE `spell_id` IN (-51664, 51664, 51665, 51667)
    AND `ScriptName` IN ('spell_rog_cut_to_the_chase', 'aura_ascension_cut_to_the_chase',
        'aura_area52_cut_to_the_chase');

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (-51664, 'aura_area52_cut_to_the_chase');

UPDATE `spell_proc`
SET `SpellFamilyName` = 0, `SpellFamilyMask0` = 0, `SpellFamilyMask1` = 0, `SpellFamilyMask2` = 0,
    `ProcFlags` = 0x15510, `SpellTypeMask` = 7, `SpellPhaseMask` = 2
WHERE `SpellId` IN (-51664, 51664, 51665, 51667);
