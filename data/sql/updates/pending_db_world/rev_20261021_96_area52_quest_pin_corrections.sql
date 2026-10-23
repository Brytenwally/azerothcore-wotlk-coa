-- Map-only upstream corrections: 2183a756af, 8b2683012d, 23cd6a444b.
-- Sacred Flame uses item objective slot 2 (ObjectiveIndex 5); preserve other markers.
-- SuperTrack arrow streaming is separate and is not changed by this migration.
UPDATE `quest_poi_points` SET `X`=-8330,`Y`=280 WHERE `QuestID`=2745 AND `Idx1`=0 AND `Idx2`=0;
UPDATE `quest_poi_points` SET `X`=-8279,`Y`=227 WHERE `QuestID`=337 AND `Idx1`=2 AND `Idx2`=0;
UPDATE `quest_poi` SET `ObjectiveIndex` = 5 WHERE `QuestID` = 1195 AND `id` = 1;
DELETE FROM `quest_poi_points` WHERE `QuestID` = 1195 AND `Idx1` = 1;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`) VALUES
(1195, 1, 0, 1884, -1772),
(1195, 1, 1, 1880, -1762),
(1195, 1, 2, 1870, -1758),
(1195, 1, 3, 1861, -1762),
(1195, 1, 4, 1856, -1772),
(1195, 1, 5, 1861, -1781),
(1195, 1, 6, 1870, -1786),
(1195, 1, 7, 1880, -1781);
