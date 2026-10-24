CREATE TABLE `area52_test_boost_campaign` (
  `id` INT UNSIGNED NOT NULL PRIMARY KEY,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `use_enabled` TINYINT UNSIGNED NOT NULL DEFAULT 0,
  `new_character_guid_floor` INT UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
CREATE TABLE `area52_test_boost_legacy_account` (
  `account` INT UNSIGNED NOT NULL PRIMARY KEY
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
INSERT INTO `area52_test_boost_legacy_account` (`account`) SELECT `account` FROM `coa_account_warchest`;
INSERT INTO `area52_test_boost_campaign` (`id`,`enabled`,`use_enabled`,`new_character_guid_floor`)
SELECT 1,1,1,COALESCE(MAX(`guid`),0) FROM `characters`;
