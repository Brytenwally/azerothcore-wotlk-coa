-- rev_20261018_01 writes worldforged_upgrade.unlock_creature, a column the live Area 52 database has but
-- mod-worldforged-upgrades never created, so a world database built from the CoA world package stopped there.
-- Added only when missing, the way rev_20261017_10 adds ethereal_bazaar_set_cache.count.
SET @has_unlock_creature := (SELECT COUNT(*) FROM `information_schema`.`COLUMNS` WHERE `TABLE_SCHEMA` = DATABASE() AND `TABLE_NAME` = 'worldforged_upgrade' AND `COLUMN_NAME` = 'unlock_creature');
SET @add_unlock_creature := IF(@has_unlock_creature = 0, 'ALTER TABLE `worldforged_upgrade` ADD COLUMN `unlock_creature` INT UNSIGNED NOT NULL DEFAULT 0', 'DO 0');
PREPARE add_unlock_creature FROM @add_unlock_creature;
EXECUTE add_unlock_creature;
DEALLOCATE PREPARE add_unlock_creature;
