-- Area 52 Freepick: characters tables the Area 52 code uses that no migration created. Re-runnable.

-- Build Creator storage (schema from branch feat/area52-build-storage).
CREATE TABLE IF NOT EXISTS `area52_build` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `owner_account` int unsigned NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `archived` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  KEY `owner_archive` (`owner_account`, `archived`, `id`),
  CONSTRAINT `area52_build_archive_valid` CHECK (`archived` IN (0, 1))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_build_revision` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `format_version` smallint unsigned NOT NULL DEFAULT 1,
  `category` int unsigned NOT NULL,
  `name` varchar(128) NOT NULL,
  `document` json NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`, `revision`),
  KEY `category_build` (`category`, `build_id`, `revision`),
  CONSTRAINT `area52_revision_build` FOREIGN KEY (`build_id`) REFERENCES `area52_build` (`id`) ON DELETE CASCADE,
  CONSTRAINT `area52_revision_positive` CHECK (`revision` > 0 AND `format_version` > 0),
  CONSTRAINT `area52_document_object` CHECK (JSON_TYPE(`document`) = 'OBJECT'),
  CONSTRAINT `area52_document_size` CHECK (JSON_STORAGE_SIZE(`document`) <= 65536)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_build_featured` (
  `build_id` bigint unsigned NOT NULL,
  `revision` int unsigned NOT NULL,
  `featured_by_account` int unsigned NOT NULL,
  `featured_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`build_id`),
  CONSTRAINT `area52_featured_revision` FOREIGN KEY (`build_id`, `revision`)
    REFERENCES `area52_build_revision` (`build_id`, `revision`) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Mystic Enchants owned per account.
CREATE TABLE IF NOT EXISTS `account_freepick_mystic_collection` (
  `account` int unsigned NOT NULL,
  `spell` int unsigned NOT NULL,
  PRIMARY KEY (`account`, `spell`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Tester certification: one certified character per entry, and its rewards.
CREATE TABLE IF NOT EXISTS `area52_tester_certification` (
  `entry` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `character_name` varchar(12) NOT NULL,
  `evidence_sha256` char(64) NOT NULL DEFAULT '',
  `checks_confirmed` tinyint unsigned NOT NULL DEFAULT 0,
  `certified_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`entry`),
  KEY `account` (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_certification_last_character` (
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `character_name` varchar(12) NOT NULL,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_certification_reward_rule` (
  `account` int unsigned NOT NULL,
  `item` int unsigned NOT NULL,
  `required_count` int unsigned NOT NULL DEFAULT 1,
  PRIMARY KEY (`account`, `item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_certification_reward_delivery` (
  `account` int unsigned NOT NULL,
  `guid` int unsigned NOT NULL,
  `item` int unsigned NOT NULL,
  `delivered_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- One-time mail deliveries per character (delivered_at = unix time, 0 = still owed).
CREATE TABLE IF NOT EXISTS `area52_archetype_vanity_delivery` (
  `guid` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_tester_cache_delivery` (
  `guid` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
