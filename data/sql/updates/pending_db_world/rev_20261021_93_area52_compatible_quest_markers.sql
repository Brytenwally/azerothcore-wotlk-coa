-- Area 52: scoped compatible missing quest markers from reviewed upstream corrections.
-- Existing keys and turn-ins are preserved; targets and map mappings are checked against deployed data.
DELETE FROM `quest_poi` WHERE (`QuestID`,`id`) IN (
(9170,5),
(17002,1),
(17006,1),
(17006,2),
(100073,1),
(1660074,1),
(1660074,2));
INSERT INTO `quest_poi` (`QuestID`,`id`,`ObjectiveIndex`,`MapID`,`WorldMapAreaId`,`Floor`,`Priority`,`Flags`,`VerifiedBuild`) VALUES
(17002,1,4,0,30,0,0,1,NULL),
(17006,1,0,0,30,0,0,1,NULL),
(17006,2,4,0,30,0,0,1,NULL),
(100073,1,4,0,30,0,0,1,NULL),
(1660074,1,0,1,41,0,0,1,NULL),
(1660074,2,1,1,41,0,0,1,NULL),
(9170,5,3,530,463,0,0,1,0);
DELETE FROM `quest_poi_points` WHERE (`QuestID`,`Idx1`) IN (
(9170,5),
(17002,1),
(17006,1),
(17006,2),
(100073,1),
(1660074,1),
(1660074,2));
INSERT INTO `quest_poi_points` (`QuestID`,`Idx1`,`Idx2`,`X`,`Y`,`VerifiedBuild`) VALUES
(17002,1,0,-9275,-920,NULL),
(17002,1,1,-8985,-920,NULL),
(17002,1,2,-8985,-1175,NULL),
(17002,1,3,-9275,-1175,NULL),
(17006,1,0,-9845,-1500,NULL),
(17006,1,1,-9710,-1500,NULL),
(17006,1,2,-9710,-1640,NULL),
(17006,1,3,-9845,-1640,NULL),
(17006,2,0,-9845,-1500,NULL),
(17006,2,1,-9710,-1500,NULL),
(17006,2,2,-9710,-1640,NULL),
(17006,2,3,-9845,-1640,NULL),
(100073,1,0,-9855,-400,NULL),
(100073,1,1,-9725,-400,NULL),
(100073,1,2,-9725,-525,NULL),
(100073,1,3,-9855,-525,NULL),
(1660074,1,0,9701,1033,NULL),
(1660074,1,1,9791,951,NULL),
(1660074,1,2,9817,955,NULL),
(1660074,1,3,9837,988,NULL),
(1660074,1,4,9839,1000,NULL),
(1660074,1,5,9832,1022,NULL),
(1660074,1,6,9807,1027,NULL),
(1660074,2,0,9792,956,NULL),
(9170,5,0,6311,-6250,0);
