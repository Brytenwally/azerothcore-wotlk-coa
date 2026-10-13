-- Area 52 deployment only. Preserve the upstream spawn identity and per-character claims.
DELETE FROM `gameobject`
WHERE `guid` = 6910429 AND `id` = 1345173 AND `map` = 389
    AND `Comment` = 'Area52 Worldforged Flameburst; cache near Jergosh; placement requires in-game height check';

INSERT INTO `gameobject`
(`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`,
 `position_x`, `position_y`, `position_z`, `orientation`,
 `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`,
 `animprogress`, `state`, `ScriptName`, `Comment`)
SELECT 6910429, 1345173, 389, 0, 0, 1, 1,
    -371.200, 191.000, -21.130, 0.624764,
    0, 0, 0.307326, 0.951604, 0, 0, 1, 'worldforged_pickup',
    'Area52 Worldforged Flameburst; cache near Jergosh; placement requires in-game height check'
FROM `gameobject_template` AS `template`
WHERE `template`.`entry` = 1345173
    AND `template`.`ScriptName` = 'worldforged_pickup'
    AND EXISTS (SELECT 1 FROM `gameobject_loot_template`
        WHERE `Entry` = 1345173 AND `Item` = 100369 AND `Chance` = 100)
    AND EXISTS (SELECT 1 FROM `item_template`
        WHERE `entry` = 100369 AND `ScriptName` = 'item_area52_identified_mystic')
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `guid` = 6910429)
    AND NOT EXISTS (SELECT 1 FROM `gameobject` WHERE `id` = 1345173 AND `map` = 389);
