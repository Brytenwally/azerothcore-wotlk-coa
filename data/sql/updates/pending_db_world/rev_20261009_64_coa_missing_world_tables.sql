-- World tables that later updates and the server read, but that no migration created: the live Area 52 database
-- has them, a world database built from the CoA world package stopped at the first update writing to one.
-- Column types follow what the server reads (Creature.cpp, LootMgr.cpp, CoADungeonSpellDamage.cpp,
-- XPHotspots.cpp) and what the updates write. CREATE TABLE IF NOT EXISTS leaves existing tables alone.

CREATE TABLE IF NOT EXISTS `coa_dungeon_health` (
  `map_id` SMALLINT UNSIGNED NOT NULL,
  `difficulty` TINYINT UNSIGNED NOT NULL,
  `creature_entry` INT UNSIGNED NOT NULL,
  `max_health` INT UNSIGNED NOT NULL,
  `evidence` VARCHAR(32) NOT NULL DEFAULT '',
  `source` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`map_id`, `difficulty`, `creature_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_dungeon_loot_variant` (
  `base_item` INT UNSIGNED NOT NULL,
  `heroic_item` INT UNSIGNED NOT NULL,
  `mythic_item` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`base_item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_dungeon_spell_damage` (
  `spell_id` INT UNSIGNED NOT NULL,
  `multiplier` FLOAT NOT NULL DEFAULT 1,
  PRIMARY KEY (`spell_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_dungeon_bosses` (
  `entry` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_dungeon_creature_damage` (
  `entry` INT UNSIGNED NOT NULL,
  `multiplier` FLOAT NOT NULL DEFAULT 1,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_dungeon_creature_heal` (
  `entry` INT UNSIGNED NOT NULL,
  `heal_pct` FLOAT NOT NULL DEFAULT 0,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `xp_hotspot_location` (
  `id` INT UNSIGNED NOT NULL,
  `payload` LONGTEXT NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `xp_hotspot_setting` (
  `id` INT UNSIGNED NOT NULL,
  `value` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_client_shapeshift_form` (
  `ID` INT UNSIGNED NOT NULL,
  `BonusActionBar` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_client_vanity_collection` (
  `ItemID` INT UNSIGNED NOT NULL,
  `DonationPointCost` INT UNSIGNED NOT NULL DEFAULT 0,
  `SeasonalPointCost` INT UNSIGNED NOT NULL DEFAULT 0,
  `BazaarTokenCost` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `gameobject_scale` (
  `guid` INT UNSIGNED NOT NULL,
  `scale` FLOAT NOT NULL DEFAULT 1,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `manastorm_scaling_curve` (
  `depth` INT UNSIGNED NOT NULL,
  `health_multiplier` FLOAT NOT NULL DEFAULT 1,
  `global_damage_multiplier` FLOAT NOT NULL DEFAULT 1,
  `melee_multiplier` FLOAT NOT NULL DEFAULT 1,
  `spell_multiplier` FLOAT NOT NULL DEFAULT 1,
  PRIMARY KEY (`depth`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `manastorm_group_scaling` (
  `min_members` TINYINT UNSIGNED NOT NULL,
  `max_members` TINYINT UNSIGNED NOT NULL,
  `health_multiplier` FLOAT NOT NULL DEFAULT 1,
  `global_damage_multiplier` FLOAT NOT NULL DEFAULT 1,
  `melee_multiplier` FLOAT NOT NULL DEFAULT 1,
  `spell_multiplier` FLOAT NOT NULL DEFAULT 1,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (`min_members`, `max_members`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
