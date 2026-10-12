DELETE FROM `creature` WHERE `guid` BETWEEN 9890200 AND 9890211;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `Comment`) VALUES
(9890200, 967254, 1, 1637, 10228, 1, 1438.670597, -4429.493141, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890201, 967254, 1, 1637, 10228, 1, 1439.872039, -4431.379020, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890202, 967254, 1, 1637, 10228, 1, 1437.882759, -4431.585821, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890203, 967254, 1, 1637, 10228, 1, 1441.073480, -4433.264899, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890204, 967254, 1, 1637, 10228, 1, 1439.084200, -4433.471700, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890205, 967254, 1, 1637, 10228, 1, 1437.094920, -4433.678501, 25.46259, 1.6743821, 120, 'Area52 temporary Orgrimmar AoE test group 1'),
(9890206, 967254, 1, 1637, 10228, 1, 1429.692527, -4355.927446, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2'),
(9890207, 967254, 1, 1637, 10228, 1, 1428.298252, -4354.179305, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2'),
(9890208, 967254, 1, 1637, 10228, 1, 1430.254475, -4353.763141, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2'),
(9890209, 967254, 1, 1637, 10228, 1, 1426.903977, -4352.431163, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2'),
(9890210, 967254, 1, 1637, 10228, 1, 1428.860200, -4352.015000, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2'),
(9890211, 967254, 1, 1637, 10228, 1, 1430.816423, -4351.598837, 25.46259, 4.9220023, 120, 'Area52 temporary Orgrimmar AoE test group 2');

DELETE FROM `creature_formations` WHERE `memberGUID` BETWEEN 9890200 AND 9890211;
INSERT INTO `creature_formations` (`leaderGUID`, `memberGUID`, `dist`, `angle`, `groupAI`, `point_1`, `point_2`) VALUES
(9890200, 9890200, 0, 0, 0, 0, 0),
(9890200, 9890201, 0, 0, 0, 0, 0),
(9890200, 9890202, 0, 0, 0, 0, 0),
(9890200, 9890203, 0, 0, 0, 0, 0),
(9890200, 9890204, 0, 0, 0, 0, 0),
(9890200, 9890205, 0, 0, 0, 0, 0),
(9890206, 9890206, 0, 0, 0, 0, 0),
(9890206, 9890207, 0, 0, 0, 0, 0),
(9890206, 9890208, 0, 0, 0, 0, 0),
(9890206, 9890209, 0, 0, 0, 0, 0),
(9890206, 9890210, 0, 0, 0, 0, 0),
(9890206, 9890211, 0, 0, 0, 0, 0);
