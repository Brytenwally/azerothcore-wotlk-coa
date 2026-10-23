-- Those Who Fell: upstream a5887254be and 50dc4e5993, preserving unrelated creature flags.
UPDATE `creature_template` SET `unit_flags` = `unit_flags` & ~256 WHERE `entry` = 161791;

-- No template or spawn addon exists in the qualified Area 52 baseline.
DELETE FROM `creature_template_addon` WHERE `entry` = 161791;
INSERT INTO `creature_template_addon` (`entry`,`path_id`,`mount`,`bytes1`,`bytes2`,`emote`,`visibilityDistanceType`,`auras`)
VALUES (161791,0,0,0,0,431,0,'256478');
