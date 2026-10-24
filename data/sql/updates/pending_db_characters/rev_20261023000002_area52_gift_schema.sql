CREATE TABLE IF NOT EXISTS `area52_account_gift_campaign` (
  `id` int unsigned NOT NULL,
  `enabled` tinyint unsigned NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `area52_account_gift_delivery` (
  `campaign` int unsigned NOT NULL,
  `account` int unsigned NOT NULL,
  `receiver` int unsigned NOT NULL,
  `item_guid` int unsigned NOT NULL,
  `mail_id` int unsigned NOT NULL,
  `delivered_at` int unsigned NOT NULL,
  PRIMARY KEY (`campaign`, `account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT IGNORE INTO `area52_account_gift_campaign` (`id`, `enabled`) VALUES (657112, 0);
