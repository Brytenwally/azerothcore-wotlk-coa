-- Two more world tables that updates write but no migration created (see rev_20261009_64). The columns of
-- coa_dungeon_boss_kit follow CoADungeonBossKit.cpp; manastorm_treasure_keeper_rule holds the two columns that
-- zz_manastorm_treasure_keeper_attackable updates. CREATE TABLE IF NOT EXISTS leaves existing tables alone.

CREATE TABLE IF NOT EXISTS `coa_dungeon_boss_kit` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `entry` INT UNSIGNED NOT NULL,
  `spell` INT UNSIGNED NOT NULL,
  `event` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `difficulty` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `initial_min` INT UNSIGNED NOT NULL DEFAULT 0,
  `initial_max` INT UNSIGNED NOT NULL DEFAULT 0,
  `repeat_min` INT UNSIGNED NOT NULL DEFAULT 0,
  `repeat_max` INT UNSIGNED NOT NULL DEFAULT 0,
  `health_pct` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `target` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `cast_flags` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `base_points` INT NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `entry` (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `manastorm_treasure_keeper_rule` (
  `rule_id` INT UNSIGNED NOT NULL,
  `creature_entry` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`rule_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
