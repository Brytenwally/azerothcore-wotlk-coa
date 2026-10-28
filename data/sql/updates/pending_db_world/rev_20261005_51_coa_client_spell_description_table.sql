-- coa_client_spell_description holds the spell tooltip overrides streamed to the client. Updates from
-- rev_20261005_52 onward write to it, but only rev_20261016_03 created it, so a world database built from
-- the CoA world package stopped at rev_20261005_52. Same definition as rev_20261016_03.
CREATE TABLE IF NOT EXISTS `coa_client_spell_description` (
  `ID` INT UNSIGNED NOT NULL,
  `Description` TEXT NOT NULL,
  `ToolTip` TEXT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
