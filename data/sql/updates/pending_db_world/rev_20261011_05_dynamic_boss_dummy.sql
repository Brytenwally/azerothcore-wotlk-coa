INSERT INTO `creature_template` (`entry`, `name`, `minlevel`, `maxlevel`, `exp`, `faction`, `rank`, `unit_class`, `type`, `HealthModifier`, `ArmorModifier`, `DamageModifier`, `RegenHealth`, `ScriptName`) VALUES
(9780200, 'Dynamic Boss Training Dummy', 1, 1, 0, 7, 1, 1, 10, 1000, 1, 1, 0, 'npc_advanced_training_dummy') ON DUPLICATE KEY UPDATE `ScriptName` = VALUES(`ScriptName`);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 9780200;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(9780200, 0, 28047, 1, 1);
