-- Character tables the server reads but no migration created: the live Area 52 database has them, a fresh one
-- stopped the worldserver at startup ("Table 'acore_characters.ascension_player_ticket' doesn't exist"). Columns
-- follow CoAPlayerTicket.cpp, AscensionCompat.cpp (appearance outfits) and AscensionWildcard.cpp (skill cards).
-- CREATE TABLE IF NOT EXISTS leaves existing tables alone.

CREATE TABLE IF NOT EXISTS `ascension_player_ticket` (
  `id` INT UNSIGNED NOT NULL,
  `account` INT UNSIGNED NOT NULL DEFAULT 0,
  `creator_guid` INT UNSIGNED NOT NULL DEFAULT 0,
  `creator` VARCHAR(64) NOT NULL DEFAULT '',
  `title` VARCHAR(255) NOT NULL DEFAULT '',
  `category` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `priority` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `affected_character` VARCHAR(64) NOT NULL DEFAULT '',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `assigned_to` VARCHAR(64) NOT NULL DEFAULT '',
  `closed_by_creator` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `created` INT UNSIGNED NOT NULL DEFAULT 0,
  `closed` INT UNSIGNED NOT NULL DEFAULT 0,
  `locale` VARCHAR(8) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `ascension_player_ticket_message` (
  `ticket` INT UNSIGNED NOT NULL,
  `id` INT UNSIGNED NOT NULL,
  `from_gm` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `gm_only` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `sender` VARCHAR(64) NOT NULL DEFAULT '',
  `message` TEXT NOT NULL,
  `created` INT UNSIGNED NOT NULL DEFAULT 0,
  `read_by` VARCHAR(64) NOT NULL DEFAULT '',
  `read_at` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`ticket`, `id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_appearance_outfit` (
  `guid` INT UNSIGNED NOT NULL,
  `name` VARCHAR(64) NOT NULL,
  `appearances` TEXT NOT NULL,
  PRIMARY KEY (`guid`, `name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_wildcard_skill_card` (
  `account` INT UNSIGNED NOT NULL,
  `card` INT UNSIGNED NOT NULL,
  `progress` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account`, `card`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_wildcard_skill_card_pending` (
  `account` INT UNSIGNED NOT NULL,
  `id` INT UNSIGNED NOT NULL,
  `card` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`account`, `id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_wildcard_skill_card_account` (
  `account` INT UNSIGNED NOT NULL,
  `bonus_progress` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_wildcard_skill_card_purchase` (
  `account` INT UNSIGNED NOT NULL,
  `type` TINYINT UNSIGNED NOT NULL,
  `count` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account`, `type`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `coa_wildcard_specialization_cache` (
  `account` INT UNSIGNED NOT NULL,
  `claimed_at` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
