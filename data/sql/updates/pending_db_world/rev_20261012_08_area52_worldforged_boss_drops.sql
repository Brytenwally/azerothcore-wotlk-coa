-- Area 52 source scroll acquisition. Owner policy: 100%, one shared loot item per mapped scroll.
-- Great Cat Spirit uses a shared generic loot table: isolate it before adding its specific reward.
DELETE FROM `creature_loot_template` WHERE `Entry` = 990115;
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT 990115, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`
FROM `creature_loot_template` WHERE `Entry` = 1978660000;
UPDATE `creature_template` SET `lootid` = 990115 WHERE `entry` = 11957 AND `lootid` IN (1978660000, 990115);

DELETE FROM `creature_loot_template` WHERE `Item` = 100369 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11518 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 100369, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Flameburst'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11518 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132623 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11517 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132623, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Ragefire Cleave'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11517 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132624 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11520 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132624, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Ragefire Nova'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11520 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132635 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3886 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132635, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Piercing Cleaver'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3886 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132595 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3975 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132595, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Herod''s Legacy'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3975 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132583 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3983 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132583, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Interrogator''s Immolation'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3983 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132690 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4543 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132690, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Veil of Shadows'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4543 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132701 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4829 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132701, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Hydra''s Venom'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4829 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132709 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4832 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132709, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Twilight Shock'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4832 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132702 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 1716 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132702, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Knifestorm'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 1716 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132638 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 7354 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132638, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Shadow Word: Gore'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 7354 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132598 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 9019 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132598, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Shield Of The Emperor'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 9019 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132432 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10432 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132432, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Shadow Dagger'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10432 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132609 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11622 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132609, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Bone Spear'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11622 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132431 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 12258 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132431, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Razorlash'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 12258 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132689 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11957 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132689, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Lunar Aspiration'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11957 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132580 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 6487 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132580, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Arcanist''s Explosion'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 6487 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132724 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3470 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132724, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Fire And Fury'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3470 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132424 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3669 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132424, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Serpent Form'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3669 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 134409 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10828 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 134409, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Consumed By Shadows'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10828 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132713 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11496 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132713, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Void Zone'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11496 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132602 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10198 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132602, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Megafrost'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10198 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132703 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4275 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132703, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Arugal''s Blink'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 4275 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132715 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10813 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132715, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Carrion Swarm'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10813 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132649 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3654 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132649, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Devour Mind'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3654 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132693 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11502 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132693, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Firefall'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11502 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132717 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10439 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132717, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Flesh Hook'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10439 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132658 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3887 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132658, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Ghostly Blow'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 3887 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132633 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11502 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132633, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Mayhem'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 11502 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132634 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 7355 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132634, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Spiderbite'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 7355 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 120573 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10439 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 120573, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Venom Belch'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 10439 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132636 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 12876 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132636, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Water Geyser'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 12876 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;

DELETE FROM `creature_loot_template` WHERE `Item` = 132652 AND `Entry` IN (
SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 1853 AND `c`.`lootid` NOT IN (0, 1978660000));
INSERT INTO `creature_loot_template`
(`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
SELECT `lootid`, 132652, 0, 100, 0, 1, 0, 1, 1, 'Area52 Worldforged: Darkmaster'
FROM (SELECT DISTINCT `c`.`lootid` FROM `creature_template` AS `b` JOIN `creature_template` AS `c`
    ON `c`.`entry` IN (`b`.`entry`, `b`.`difficulty_entry_1`, `b`.`difficulty_entry_2`, `b`.`difficulty_entry_3`)
    WHERE `b`.`entry` = 1853 AND `c`.`lootid` NOT IN (0, 1978660000)) AS `sources`;
