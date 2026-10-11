# CoA DBC tools

## Agent retrieval: coa-dbc-viewer

`coa-dbc-viewer` is a read-only, JSON-first tool for agents investigating CoA abilities. Requires Python 3.11+;
inspection needs no third-party Python packages, browser, server build or running worldserver. `source` uses `rg`.
Pass the actual client extraction or server `DataDir/dbc` explicitly; these are different evidence sources.
The optional browser view uses the same parser. No game data is committed or uploaded.

For ranks/acquisition, interpreted execution paths and verification links, use the
[spell and quest mechanic map](../coa-mechanics/README.md): `coa-dbc-viewer map --data <directory> --id <spell>`.
`record --with-map` embeds it alongside selected raw fields; `--details` expands its linked contracts.

```sh
python apps/coa-dbc/coa-dbc-viewer tables --data env/dist/data/dbc
python apps/coa-dbc/coa-dbc-viewer search --data env/dist/data/dbc --table Spell --query Frostbolt --limit 10
python apps/coa-dbc/coa-dbc-viewer schema --data env/dist/data/dbc --table Spell
python apps/coa-dbc/coa-dbc-viewer record --data env/dist/data/dbc --id 116 \
  --field 'SpellName[enUS]' --field 'Description[enUS]' --field 'Effect*' --with-links
python apps/coa-dbc/coa-dbc-viewer links --data env/dist/data/dbc --id 116
python apps/coa-dbc/coa-dbc-viewer source --data env/dist/data/dbc --id 116
python apps/coa-dbc/coa-dbc-viewer compare --data env/dist/data/dbc --other out/client-dbc --id 116
```

- Successful retrieval writes JSON to stdout. Failures return nonzero with JSON errors on stderr (argument
  syntax errors use argparse). `--field` matches an exact name or a trailing `*` prefix; unknown names fail.
- Search supports name/ID substrings, `--locale`, `--offset` and `--limit` (maximum 200). Use `record --id`
  for exact ID lookup. Spell records include relevant scenario definitions and their registered numerical
  checks; these links do not claim a scenario has run. `source` finds text references, not implementation proof.
- Typed fields include raw bytes, signed values, floats, localized strings, known flag labels and links.
  Provenance includes file/schema-source hashes and extraction-manifest agreement. A missing manifest means
  unknown archive origin; a mismatched one is marked stale. A directory path does not establish client version.
- Reviewed layouts cover Spell, SpellDuration, SpellRadius, SpellRange, SpellCastTimes, Talent, TalentTab,
  SkillLineAbility, CharacterAdvancement, CharacterAdvancementClassTypes, CharacterAdvancementTabTypes,
  ChrSpecs, SpellCharges, SpellChargesCategory, SpellAddon and SpellCustomAttr. Some custom layouts are partial:
  unnamed bytes remain opaque. Other tables support `record --table NAME --row 0` as raw bytes without guessed IDs.
- Truncated files, duplicate IDs and unexpected reviewed layouts fail. Bad strings/nonfinite floats carry
  field errors. CoA's packed CharacterAdvancement has 179 logical fields in 692 bytes, so fields are byte-addressed.
- `compare` reports semantic changes, including client descriptions, while ignoring string-block repacking.
  It compares a selected ID present in both data sets. Use `client_dbc.py diff` for whole-set core-visible changes.
- DBC values precede SQL and C++ corrections. In particular, `EffectBonusMultiplier` is not the effective CoA
  AP/SP coefficient. Incoming references show acquisition candidates; trace their conditions before concluding
  that players can acquire or execute an ability. Cached data rejects detected file changes; restart after replacement.

Optional evidence layers keep database rows and recorded gameplay separate from DBC fields:

```sh
python apps/coa-dbc/capture_sql.py --config path/to/worldserver.conf --mysql path/to/mysql \
  --spell 116 --output out/spell-sql.json
python apps/coa-dbc/coa-dbc-viewer record --data env/dist/data/dbc --id 116 \
  --sql out/spell-sql.json --observations .cache/coa-gameplay-tests/RUN
python apps/coa-dbc/coa-dbc-viewer serve --data env/dist/data/dbc --port 8765
```

SQL capture uses SELECTs on a local configured world database, follows rank chains (including negative-ID
bindings), and captures bonus/proc/rank/script/DBC/learn metadata. Credentials stay in a temporary mode-600 file.
The capture may include several requested spells and their ranks; inspect row IDs. It is a sequence of reads,
not an atomic snapshot or a calculation of effective runtime values. Result directories must contain matching
scenario, summary and native result identities. Recorded samples remain historical evidence, including failures.
Neither evidence layer proves current deployment or rendered-client behavior. The optional HTTP API binds only
to loopback and is read-only: `/api/tables`, `/api/search?table=Spell&q=Frostbolt`, `/api/record?table=Spell&id=116`.

Tests: `python -B apps/coa-dbc/test_inspector.py` and `python -B apps/coa-dbc/test_capture_sql.py`.
The byte fixtures independently check signed/string/packed decoding, invalid layouts, reference cycles,
stale provenance, semantic differences and evidence mismatches. Keep new field definitions grounded in a
reviewed native reader; a plausible label is not enough.

## Client extraction and whole-set checks

The worldserver loads `DataDir/dbc` at startup. It must hold the original CoA client's tables: with a stock
or partial set it drops content that references CoA-only rows, such as item limit categories, gameobjects and
currencies. Players use the same tables, so client patches must not replace DBC archives, and `*_dbc` world
tables must not override client rows. Game files are never committed.

Extraction additionally requires [mpqcli](https://github.com/TheGrayDot/mpqcli).

```sh
python apps/coa-dbc/client_dbc.py extract "C:/CoA/client/Data" out/client-dbc --original --mpqcli path/to/mpqcli.exe
python apps/coa-dbc/client_dbc.py check out/client-dbc
python apps/coa-dbc/client_dbc.py diff "C:/CoA/server/data/dbc" out/client-dbc
```

Copy the extracted files into `<DataDir>/dbc` and restart the worldserver.

- `extract` reads the client's archives in load order; the last archive holding a table wins. `--original`
  reads the launcher's untouched `NAME.ORIGINAL` copies, `--archive NAME=PATH` substitutes any archive. Tables
  are named as the worldserver opens them. `client-dbc.manifest.json` records each table's archive and hash.
- `check` validates the set against this checkout's `DBCfmt.h`: missing tables, file name case and field
  layouts. Strings outside the string block and rows with index -1 are reported as notes.
- `diff` compares the values the core reads, so a rebuilt string block is not a change.

Tests: `python apps/coa-dbc/test_client_dbc.py`

## Four missing Area 52 heirloom ring profiles

The four XP-only rings gain separate level-1–80 combat-stat profiles. The world migration
`rev_20261011_00_area52_heirloom_ring_stats.sql` adds distribution IDs 12040–12043 and changes
only the four items' scaling distribution and mask. Existing XP spell 71354, item identities,
prices, bindings, quality, appearances, ownership, and inventory instances remain unchanged.

Generate the matching client table with:

```sh
python apps/coa-dbc/patch_area52_heirloom_rings.py original/ScalingStatDistribution.dbc candidate/ScalingStatDistribution.dbc
python -B tools/verify_all.py --stages harness --harness test_area52_heirloom_rings
```

The patcher preserves existing records and rejects ID collisions. Include its output in the
effective Area 52 client overlay before activating the server migration through the normal updater.
The server can use the identical DBC; the SQL definitions are byte-equivalent. Apply the client
overlay together with the item mappings and refresh cached item queries before comparing tooltips.
The migration advances the reviewed client-cache version from 17 to 18. With the live
`ClientCacheVersion = 0` configuration, the server sends this version on the next login to refresh
cached item definitions. If a different version or explicit configuration is active, reconcile the
cache refresh before deployment rather than relying on this conditional bump.
No server core rebuild is needed. Preflight both DBC and SQL profile IDs and item references before
installation; future conflicting definitions must be reconciled rather than overwritten.

The profiles use the already deployed Area 52 one-hand/accessory stat curve (mask 4), with a
level-80 budget of 115. They do not modify that curve or other items. Under the deployed curve:

| Ring | Level-80 stats |
| --- | --- |
| Antique Myrmidon's Signet (62035) | 39 Strength, 39 Stamina, 24 Hit, 22 Crit |
| Ornate Band of Accuria (62036) | 37 Agility, 39 Stamina, 25 Crit, 25 Haste |
| Gleaming Seal of the Archmagus (62037) | 28 Intellect, 30 Stamina, 46 Spell Power, 13 Crit, 15 Haste |
| Burnished Dark Iron Ring (69891) | 23 Strength, 52 Stamina, 28 Dodge, 28 Expertise |

These are Area 52 balancing profiles, not a claim of recovered official Ascension values. They
retain distinct physical, caster, and tank roles. Explicit Spell Power keeps the caster ring useful
under the existing level-80 item model. Weighted stat totals target approximately 90% of basic
item-level 187 rare ring references, using the same point convention as the deployed category tune.
The JSON records target stats, reference item IDs/budgets, and the installed level-1–80 curve.
The current Callboard pool has no level-80 reference rewards. On a different scaling curve the
profiles still function, but the stated endpoint values and budget ratios do not apply.

Structural checks cover source/client agreement, unchanged XP fields, complete monotonic level
growth, the endpoint budget, collision rejection and idempotency. The optional temporary-table
MySQL test uses `AREA52_RING_MYSQL_CONTAINER` with the established container-local root option file;
it applies the migration twice to connection-local fixtures without changing persistent world data.
In-game tooltip/equipped-stat confirmation remains a separate activation acceptance check.
