DELETE FROM `creature` WHERE `guid` BETWEEN 9890200 AND 9890205;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `Comment`) VALUES
(9890200, 967254, 1, 1637, 10228, 1, 1538.435126, -4441.594071, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890201, 967254, 1, 1637, 10228, 1, 1538.340645, -4443.828142, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890202, 967254, 1, 1637, 10228, 1, 1536.591180, -4442.858929, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890203, 967254, 1, 1637, 10228, 1, 1538.246165, -4446.062213, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890204, 967254, 1, 1637, 10228, 1, 1536.496700, -4445.093000, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890205, 967254, 1, 1637, 10228, 1, 1534.747235, -4444.123787, 13.88309, 1.0648832, 120, 'Area52 temporary Orgrimmar AoE test group 1');

DELETE FROM `creature_formations` WHERE `memberGUID` BETWEEN 9890200 AND 9890205;
INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(9890200, 9890200, 0, 0, 0, 0, 0),
(9890200, 9890201, 0, 0, 0, 0, 0),
(9890200, 9890202, 0, 0, 0, 0, 0),
(9890200, 9890203, 0, 0, 0, 0, 0),
(9890200, 9890204, 0, 0, 0, 0, 0),
(9890200, 9890205, 0, 0, 0, 0, 0);
