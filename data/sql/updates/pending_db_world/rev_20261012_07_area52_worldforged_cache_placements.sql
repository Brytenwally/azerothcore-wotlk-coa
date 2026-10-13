-- Staged Area 52 test placements. Requires area52_worldforged_scroll_cache before deployment.
-- Coordinates are reconstructed beside normal-mode boss anchors, not recovered original cache positions.
-- Dedicated templates avoid changing upstream gear pickup behavior. No migration of character claims.

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990100, 10, 259, 'Worldforged Cache: Flameburst', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990100)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791000 AND `id` = 990100;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791000, 990100, 389, 1, 1,
    -375.7359, 212.0247, -21.801, 5.91667, 0.1822336, -0.9832553,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Flameburst; reward 100369'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990100
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791000);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990101, 10, 259, 'Worldforged Cache: Ragefire Cleave', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990101)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791001 AND `id` = 990101;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791001, 990101, 389, 1, 1,
    -144.6948, 37.7856, -38.8039, 4.39823, 0.8090169, -0.5877854,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Ragefire Cleave; reward 132623'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990101
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791001);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990102, 10, 259, 'Worldforged Cache: Ragefire Nova', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990102)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791002 AND `id` = 990102;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791002, 990102, 389, 1, 1,
    -242.0133, 148.8404, -18.7494, 4.28461, 0.8410859, -0.5409016,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Ragefire Nova; reward 132624'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990102
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791002);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990103, 10, 259, 'Worldforged Cache: Piercing Cleaver', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990103)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791003 AND `id` = 990103;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791003, 990103, 33, 1, 1,
    -200.3666, 2255.9526, 76.2846, 3.97935, 0.9135456, -0.4067363,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Piercing Cleaver; reward 132635'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990103
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791003);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990104, 10, 259, 'Worldforged Cache: Herod''s Legacy', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990104)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791004 AND `id` = 990104;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791004, 990104, 189, 1, 1,
    1964.6207, -434.5701, 6.26076, 2.98451, 0.9969172, 0.0784606,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Herod''s Legacy; reward 132595'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990104
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791004);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990105, 10, 259, 'Worldforged Cache: Interrogator''s Immolation', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990105)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791005 AND `id` = 990105;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791005, 990105, 189, 1, 1,
    1789.5336, 1124.7536, 7.57362, 4.81711, 0.6691301, -0.7431452,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Interrogator''s Immolation; reward 132583'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990105
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791005);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990106, 10, 259, 'Worldforged Cache: Veil of Shadows', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990106)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791006 AND `id` = 990106;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791006, 990106, 189, 1, 1,
    1820.3924, 1413.7105, -7.91731, 3.15905, 0.9999619, -0.0087286,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Veil of Shadows; reward 132690'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990106
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791006);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990107, 10, 259, 'Worldforged Cache: Hydra''s Venom', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990107)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791007 AND `id` = 990107;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791007, 990107, 48, 1, 1,
    -851.446, -453.865, -33.8922, 1.5708, 0.7071081, 0.7071055,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Hydra''s Venom; reward 132701'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990107
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791007);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990108, 10, 259, 'Worldforged Cache: Twilight Shock', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990108)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791008 AND `id` = 990108;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791008, 990108, 48, 1, 1,
    -815.8338, -155.4713, -25.7923, 4.74729, 0.6946604, -0.7193379,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Twilight Shock; reward 132709'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990108
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791008);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990109, 10, 259, 'Worldforged Cache: Knifestorm', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990109)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791009 AND `id` = 990109;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791009, 990109, 34, 1, 1,
    92.4223, -136.0501, -33.9396, 5.00727, 0.5955560, -0.8033138,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Knifestorm; reward 132702'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990109
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791009);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990110, 10, 259, 'Worldforged Cache: Shadow Word: Gore', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990110)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791010 AND `id` = 990110;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791010, 990110, 129, 1, 1,
    2363.8498, 907.3858, 28.7671, 0.304753, 0.1517875, 0.9884131,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Shadow Word: Gore; reward 132638'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990110
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791010);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990111, 10, 259, 'Worldforged Cache: Shield Of The Emperor', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990111)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791011 AND `id` = 990111;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791011, 990111, 230, 1, 1,
    1377.2691, -830.9192, -87.59, 1.32645, 0.6156614, 0.7880108,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Shield Of The Emperor; reward 132598'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990111
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791011);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990112, 10, 259, 'Worldforged Cache: Shadow Dagger', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990112)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791012 AND `id` = 990112;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791012, 990112, 289, 1, 1,
    143.8466, 96.1188, 104.659, 3.26377, 0.9981347, -0.0610507,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Shadow Dagger; reward 132432'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990112
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791012);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990113, 10, 259, 'Worldforged Cache: Bone Spear', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990113)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791013 AND `id` = 990113;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791013, 990113, 289, 1, 1,
    137.5625, 168.7052, 95.9664, 3.28122, 0.9975640, -0.0697570,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Bone Spear; reward 132609'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990113
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791013);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990114, 10, 259, 'Worldforged Cache: Razorlash', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990114)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791014 AND `id` = 990114;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791014, 990114, 349, 1, 1,
    981.8706, -10.4656, -62.5035, 4.60767, 0.7431446, -0.6691309,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Razorlash; reward 132431'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990114
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791014);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990115, 10, 259, 'Worldforged Cache: Lunar Aspiration', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990115)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791015 AND `id` = 990115;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791015, 990115, 1, 1, 1,
    7359.9728, -2726.1769, 461.309, 6.21553, 0.0338212, -0.9994279,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Lunar Aspiration; reward 132689'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990115
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791015);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990116, 10, 259, 'Worldforged Cache: Arcanist''s Explosion', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990116)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791016 AND `id` = 990116;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791016, 990116, 189, 1, 1,
    148.4247, -425.6938, 18.4864, 6.24828, 0.0174518, -0.9998477,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Arcanist''s Explosion; reward 132580'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990116
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791016);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990117, 10, 259, 'Worldforged Cache: Fire And Fury', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990117)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791017 AND `id` = 990117;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791017, 990117, 1, 1, 1,
    316.5332, -2238.9516, 212.109, 2.75762, 0.9816272, 0.1908091,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Fire And Fury; reward 132724'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990117
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791017);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990118, 10, 259, 'Worldforged Cache: Serpent Form', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990118)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791018 AND `id` = 990118;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791018, 990118, 43, 1, 1,
    -151.9659, 417.2508, -72.6294, 0.279253, 0.1391733, 0.9902680,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Serpent Form; reward 132424'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990118
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791018);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990119, 10, 259, 'Worldforged Cache: Consumed By Shadows', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990119)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791019 AND `id` = 990119;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791019, 990119, 0, 1, 1,
    1564.3761, -5609.9256, 114.273, 1.06465, 0.5075380, 0.8616294,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Consumed By Shadows; reward 134409'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990119
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791019);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990120, 10, 259, 'Worldforged Cache: Void Zone', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990120)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791020 AND `id` = 990120;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791020, 990120, 429, 1, 1,
    -35.0812, 812.3877, -29.4525, 4.69494, 0.7132489, -0.7009108,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Void Zone; reward 132713'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990120
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791020);

INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `data0`, `data1`, `ScriptName`)
SELECT 990121, 10, 259, 'Worldforged Cache: Megafrost', 1, 0, 0, 'area52_worldforged_scroll_cache'
WHERE NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990121)
ON DUPLICATE KEY UPDATE `entry` = VALUES(`entry`);
DELETE FROM `gameobject` WHERE `guid` = 9791021 AND `id` = 990121;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `spawnMask`, `phaseMask`,
    `position_x`, `position_y`, `position_z`, `orientation`, `rotation2`, `rotation3`,
    `spawntimesecs`, `state`, `ScriptName`, `Comment`)
SELECT 9791021, 990121, 1, 1, 1,
    5209.1326, -4813.2206, 701.077, 0.733038, 0.3583678, 0.9335805,
    0, 1, 'area52_worldforged_scroll_cache',
    'Area52 reconstructed test cache: Megafrost; reward 132602'
WHERE EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990121
    AND `ScriptName` = 'area52_worldforged_scroll_cache')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 9791021);
