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

## Area 52 high-level heirloom budgets (opt-in)

`patch_area52_heirloom_scaling.py` stages a matching client/server `ScalingStatValues.dbc`
for Area 52. It is not an automatic upstream database migration and does not install anything.

```sh
python apps/coa-dbc/patch_area52_heirloom_scaling.py /path/to/ScalingStatValues.dbc out/ScalingStatValues.dbc
python -B tools/verify_all.py --stages harness --harness test_area52_heirloom_scaling
```

The profile targets approximately 90% of basic item-level 187 rare dungeon equipment at level 80.
It changes shared category budgets at levels 68–80, preserving each item's stat distribution,
levels 1–67, armor values/types, XP bonuses, and proc spells. Primary/secondary stat budgets use
the median matched-item point ratio; weapon DPS and caster weapon spell power use separate curves.
The point convention is 1 per ordinary stat/rating, 2/3 per Stamina, 1/2 per AP/RAP, 6/7 per spell
power, and 2.5 per mana-per-five. This is an alpha balancing convention, not a combat-power guarantee.
Item mixes vary; individual items and stats can fall above or below the category target.

The calibration excludes heroic/raid, Bloodforged, random-property and random-suffix equipment.
It matches slot, armor-budget material or weapon subtype, and dominant primary stat. Duplicate
peer stat profiles count once. The live Callboard pools currently contain level-60 rewards and
are not used as level-80 references. Four XP-only vendor rings have no stat distribution and are
unchanged; 18 vendor items without a direct peer inherit their shared category curve.

| Budget | Level 80 before | After |
| --- | ---: | ---: |
| Shoulder | 97 | 203 |
| Trinket | 97 | 296 |
| One-hand stat | 56 | 115 |
| Primary | 131 | 257 |
| Ranged stat | 41 | 80 |
| Tertiary | 73 | 227 |
| Melee one-hand DPS | 94 | 108 |
| Melee two-hand DPS | 122 | 140 |
| Caster one-hand DPS | 42 | 62 |
| Caster two-hand DPS | 50 | 98 |
| Ranged DPS | 85 | 98 |
| Wand DPS | 173 | 198 |
| Caster weapon spell power | 343 | 283 |

Positive budget multipliers begin at level 68 and follow the existing curve through 80.
The caster weapon spell-power reduction phases in over 68–80; its final value is lower than the
old level-80 value, but still grows with each character level. No category drops on leveling.
Burnished Spaulders of Might consequently reach 38 Strength, 82 Stamina, 16 Hit and 27 Defense
at 80; their existing 1,256 armor is unchanged.

These are shared DBC curves. The live impact audit also identifies 18 Felforged armor pieces,
three test heirloom templates and eleven developer templates with applicable level-80 caps.
Do not represent this as an item-quality-only change. Lower-cap Battleground Spoils and Necklace
of the Sunken Empire remain unaffected. Deployment must account for that shared scope.

For installation, use the generated table in both the server's DBC directory and the effective
Area 52 client overlay, preserving all other archive entries. First check
`SELECT * FROM scalingstatvalues_dbc WHERE Charlevel BETWEEN 68 AND 80`; unexpected SQL overrides
must be reconciled before activation because they override the server DBC. The existing level-20
SQL fix is outside this patch. Hash and retain the prior client/server files for rollback, stop the
client before replacement, and activate the matched pair during the announced server restart.
Publish through the separate Area 52 launcher channel only after activation checks. A source push
does not distribute the client patch or activate these budgets.
