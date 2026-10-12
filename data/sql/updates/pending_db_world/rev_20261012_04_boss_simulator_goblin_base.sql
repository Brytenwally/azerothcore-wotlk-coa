INSERT INTO `gameobject_template`
(`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`)
SELECT 990001, `type`, `displayId`, 'Boss Simulator - Goblin Teleporter Base (3x)', `IconName`, `castBarCaption`, `unk1`, `size` * 3, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`
FROM `gameobject_template`
WHERE `entry` = 188454
AND NOT EXISTS (SELECT 1 FROM `gameobject_template` WHERE `entry` = 990001)
ON DUPLICATE KEY UPDATE `entry` = `gameobject_template`.`entry`;
