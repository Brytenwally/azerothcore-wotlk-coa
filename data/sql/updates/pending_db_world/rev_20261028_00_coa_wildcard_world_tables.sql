-- Wildcard world tables AscensionWildcard.cpp reads at startup on every realm, but no migration created: a fresh
-- world database stopped the worldserver. CREATE TABLE IF NOT EXISTS leaves existing tables alone.

CREATE TABLE IF NOT EXISTS `ascension_wildcard_tooltip_links` (
  `Talent` INT UNSIGNED NOT NULL,
  `Ability` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`Talent`, `Ability`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `ascension_wildcard_boss_marks` (
  `CreatureEntry` INT UNSIGNED NOT NULL,
  `Amount` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`CreatureEntry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
