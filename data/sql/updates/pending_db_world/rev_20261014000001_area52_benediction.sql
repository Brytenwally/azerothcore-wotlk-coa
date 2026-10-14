DELETE FROM `spell_script_names`
WHERE `spell_id` IN (20101, 20102, 20103) AND `ScriptName` = 'aura_area52_benediction';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(20101, 'aura_area52_benediction'),
(20102, 'aura_area52_benediction'),
(20103, 'aura_area52_benediction');

DELETE FROM `spell_script_names`
WHERE `spell_id` = 411116 AND `ScriptName` = 'spell_area52_benediction_party';

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`)
VALUES (411116, 'spell_area52_benediction_party');
