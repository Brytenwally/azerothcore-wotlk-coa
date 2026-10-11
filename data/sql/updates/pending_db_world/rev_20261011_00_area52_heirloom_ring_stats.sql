DELETE FROM `scalingstatdistribution_dbc` WHERE `ID` IN (12040, 12041, 12042, 12043);
INSERT INTO `scalingstatdistribution_dbc` (`ID`, `StatID_1`, `StatID_2`, `StatID_3`, `StatID_4`, `StatID_5`, `StatID_6`, `StatID_7`, `StatID_8`, `StatID_9`, `StatID_10`, `Bonus_1`, `Bonus_2`, `Bonus_3`, `Bonus_4`, `Bonus_5`, `Bonus_6`, `Bonus_7`, `Bonus_8`, `Bonus_9`, `Bonus_10`, `Maxlevel`) VALUES
(12040, 4, 7, 31, 32, -1, -1, -1, -1, -1, -1, 3392, 3392, 2087, 1914, 0, 0, 0, 0, 0, 0, 80),
(12041, 3, 7, 32, 36, -1, -1, -1, -1, -1, -1, 3218, 3392, 2174, 2174, 0, 0, 0, 0, 0, 0, 80),
(12042, 5, 7, 45, 32, 36, -1, -1, -1, -1, -1, 2435, 2609, 4000, 1131, 1305, 0, 0, 0, 0, 0, 80),
(12043, 4, 7, 13, 37, -1, -1, -1, -1, -1, -1, 2000, 4522, 2435, 2435, 0, 0, 0, 0, 0, 0, 80);
UPDATE `item_template` SET `ScalingStatDistribution` = 12040, `ScalingStatValue` = 4 WHERE `entry` = 62035;
UPDATE `item_template` SET `ScalingStatDistribution` = 12041, `ScalingStatValue` = 4 WHERE `entry` = 62036;
UPDATE `item_template` SET `ScalingStatDistribution` = 12042, `ScalingStatValue` = 4 WHERE `entry` = 62037;
UPDATE `item_template` SET `ScalingStatDistribution` = 12043, `ScalingStatValue` = 4 WHERE `entry` = 69891;
UPDATE `version` SET `cache_id` = 18 WHERE `cache_id` = 17;
