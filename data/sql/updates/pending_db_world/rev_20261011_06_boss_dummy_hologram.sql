UPDATE `creature_template` SET `name` = 'Boss Simulator' WHERE `entry` = 9780200;

DELETE FROM `creature_template_model` WHERE `CreatureID` = 9780200;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(9780200, 0, 18031, 1, 1);

DELETE FROM `creature_equip_template` WHERE `CreatureID` = 9780200;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `ItemID2`, `ItemID3`, `VerifiedBuild`) VALUES
(9780200, 1, 29484, 0, 0, 0);
