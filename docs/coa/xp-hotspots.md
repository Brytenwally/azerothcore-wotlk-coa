# XP hotspots

An optional, realm-controlled monster-kill XP rotation for the shared Ascension client. It does not
depend on CoA class selection, Area 52 progression, or a separately enabled addon. The server owns
eligibility, geometry, rewards, permissions, schedules and persisted GM changes. The built-in map
extension displays the server's data and submits editing requests.

## Installation

Build this server revision and let the normal world database updater apply
`rev_20261011_00_hotspots.sql` and `rev_20261011_02_hotspot_boundaries.sql`. They use only
`xp_hotspot_location` and `xp_hotspot_setting`. Initial
inserts preserve an existing location with the same ID, including GM changes and rotation state.

The feature defaults to disabled. Add the following to the realm's loaded configuration and restart:

```ini
XPHotspots.Enable = 1
XPHotspots.Brand = ""
XPHotspots.MaxContinent = 2
XPHotspots.ActiveCount = 3
XPHotspots.BuffSpell = 993945
```

`MaxContinent` is 0 for Azeroth, 1 for Azeroth and Outland, or 2 for all three continents. The effective
ceiling cannot exceed the core `Expansion` setting. An Azeroth-only progression realm using an
expansion-2 client should explicitly set this option to 0. Blood elf and draenei starting zones on
physical map 530 count as Azeroth. Instances, battlegrounds and unsupported physical maps are
excluded. Enable, map ceiling and buff changes require a restart. Active count is the startup fallback;
saved GM General Settings override it and apply without a restart.
Both map panels use a dark-blue/gold theme with neutral XP Hotspot titles. The client ignores the
legacy server brand label. GMs open the manager from **XP Hotspot Menu** in the world map
magnifying-glass filter menu. Clicking a hotspot opens its details, labels its ID in the manager,
and marks the selected pin and browser row. Existing map filters remain available.

The original buff 993945 must exist with a monster-kill-only XP aura. Startup refuses to enable the
service when the configured spell is incompatible. While enabled, that spell is reserved for hotspot
ownership and cannot be saved as a character aura. Logout removes it; login and presence checks
restore it only when appropriate. No DBC or Extensions.dll replacement is included.

### Shared client package

Use Python 3.11+ and [mpqcli](https://github.com/thegraydot/mpqcli), already used by the repository's
DBC extraction tools. The packaging path was developed against **mpqcli v0.12.0**. Its official
Windows x86-64 executable has SHA-256
`20dc64097026a1535c8586f618f3acfb0f4a9e96270ad6f959ac5d2fa4db9179`.

```powershell
python tools/package_hotspot_client.py `
  --baseline "C:/Client/Data/patch-B.MPQ" `
  --output "C:/Staging/patch-B.MPQ" `
  --mpqcli "C:/Tools/mpqcli.exe"
```

Supply the release's **effective archive containing FrameXML.toc**, including its existing realm
overlays. The tool copies that archive, adds `Interface/FrameXML/XPHotspots.lua`, and appends one
loader entry without replacing any other loader content. It extracts and compares every member
before returning the candidate and its hash report. It refuses in-place writes or existing output.
It requires no private Area 52 packaging library. Rebuild from the final combined baseline when
other client changes are being released at the same time.

Distribute the resulting archive through the client release's ordinary hash-checked launcher
manifest, alongside the matching server. A source merge does not distribute this archive. Close
the client before installing it, preserve the previous archive for rollback, and update the release
manifest hash using the launcher's normal release process. The client source and build tool belong
in this PR; proprietary baseline archives do not.

The extension reuses `hotspot-xp`, `C_WorldMap` coordinate conversion and `C_Serialize` from the
existing shared client. It hides its controls when the connected server reports the feature disabled.
Normal players cannot open the GM editor or mutate server state by forging UI requests.

## Default behavior

- Locations default to **+150% monster-kill XP**, a one-hour duration, weight 1 and a one-hour
  post-event cooldown. +150% means 2.5 times the eligible kill XP before other existing multipliers.
- Three locations rotate concurrently by default, with at most one active location per zone.
  Weighted selection applies only to enabled, valid locations permitted by the realm's map ceiling.
- Entering or leaving an area updates the buff within 500 ms. The kill-reward hook checks presence
  again before applying XP, so crossing a boundary immediately before a kill cannot retain a stale
  bonus. Existing party XP rules and no-bonus-XP challenges are preserved.
- Overlapping locations use the highest active bonus once. Quest and profession rewards do not
  receive this bonus. Remaining event duration is shown in map tooltips using server time.
- Absolute start/end times survive restart. Expired events do not restart with a fresh hour.
  Announcements are emitted for newly started events, not for restored active events.
- A successful database commit precedes publication of a GM edit or rotation. Concurrent edits use
  a revision check. Drafts and request throttling are isolated by character GUID, including two
  characters on one account. Saving a draft preserves newer runtime scheduling state.

## Map and GM editor

GMs open the world map magnifying-glass menu and choose **XP Hotspot Menu**. Hover a marker to show its server-provided name, bonus,
remaining time and coverage highlight. Circle and polygon coverage comes from the stored geometry.
Whole-zone membership uses the server's actual zone ID. Its map preview is a sampled 64-by-64 terrain
mask, generated incrementally and cached; it is an approximate visual boundary, not the XP eligibility
calculation. Missing terrain/map coverage is labelled unavailable rather than drawn as a rectangle.

For a Game Master:

1. **Browse / active** opens a paginated catalog with live, debounced search by name or exact ID. **This map / all
   locations** also exposes records outside the current map, including disabled/unresolved records.
   Catalog pages contain 12 locations to bound packet volume. Disabled icons remain selectable there.
2. Select a marker or catalog row, then **Edit selected**. Change the name, bonus, duration, rotation
   weight, cooldown, icon visibility or highlight visibility. **Save draft** submits the settings
   together. A stale edit is rejected rather than overwriting another GM's work.
3. With no draft open, CTRL-click a zone map to create a circular draft at that point. **New Draft**
   and **Save draft** sit together at the top. **New Draft** leaves saved locations unchanged and starts
   fresh with the server defaults. Choose **Polygon** to turn a new circle into a polygon starting at
   its center, or **Whole zone** for zone coverage. A shape can also be chosen before placing the center.
   Existing saved shapes are edited through **Edit selected**. Additional CTRL-clicks
   set a circle center or add polygon vertices. ALT-click selects the nearest existing vertex;
   CTRL-click moves that vertex. CTRL-right-click undoes the last vertex. Use the radius field for circles.
   Edit an existing boundary through **Edit selected**, then the same map controls.
4. Save, then **Enable selected** to admit it to rotation. **Move to next** prioritizes it for the next
   available slot, bypassing its cooldown but preserving the realm and same-zone restrictions. A
   later priority request moves ahead of an earlier one. It does not interrupt an active event.
5. **Disable selected** removes a location from rotation and ends its event. **Stop selected** ends
   its event but leaves it eligible after cooldown. **Announcements** toggles global start messages.
6. SHIFT-click 2–8 non-zone catalog rows, then **Merge selection**. All must belong to the same map
   and zone. Preview the union and set its name/settings before saving. Saving atomically retires the
   source entries and creates one disabled entry; enable it separately. The union preserves the source
   boundaries and does not fill gaps. Up to eight primitive boundaries are supported, including when
   merging an existing union. Stale source revisions reject the entire operation. Cancel leaves sources
   unchanged. Retired records stay in the database for provenance but cannot rotate or be re-enabled.

### General Settings

The **General Settings** tab manages the total active count (0–20), maximum active counts for
Eastern Kingdoms, Kalimdor, Outland and Northrend, and maximums for Alliance, Horde and
neutral/contested areas. Quotas are caps, not reserved minimums; 0 excludes a group, 20 leaves it
unrestricted within the global limit. Total 0 pauses rotation. The scheduler keeps at most one
active hotspot per zone, and realm expansion restrictions always apply. Blood elf starting zones
count toward Eastern Kingdoms; draenei starting zones count toward Kalimdor. Faction classification
uses the zone's AreaTable territory team; it never restricts the receiving player's faction.

Lower limits stop excess events on the next scheduler tick (within one second), retaining active
locations in ascending ID order when necessary. Raised limits fill from eligible weighted locations;
insufficient candidates can leave slots empty. Stopped events receive their normal cooldown.

Defaults include XP bonus (0–1000%), duration (1–1440 minutes), cooldown (0–10080 minutes), and a
global announcement switch. New locations, including merges, inherit the defaults. Existing locations
keep their values unless **Apply defaults to all existing hotspots** is explicitly checked. That option
updates XP, duration and cooldown for every non-retired location, including disabled and active ones;
it preserves geometry, eligibility, weight and queue state. Active end times are recalculated from their
original start and may expire immediately. The checkbox clears after a successful save.

All settings and optional bulk updates commit atomically to the existing `xp_hotspot_setting` and
`xp_hotspot_location` tables; no schema migration is needed. IDs 1–13 store announcements, total,
bonus, duration seconds, cooldown seconds, four continent caps, three faction caps, and revision.
They survive restarts. Concurrent GM saves reject stale revisions; **Reload** retrieves current values.
The protocol fallback `.hotspot general` reads settings. To save, pass those 13 integer values in order,
followed by 0 or 1 for applying defaults. Both entry points require GM authorization.

Additional map-editor protocol commands are `newmap KIND MAP ZONE X Y NAME`, `vertex INDEX X Y`
and `merge ID REVISION ID REVISION ...`. They have the same GM authorization as the text fallback.

The text command fallback uses the same server validation and requires GM security and the existing
event-start command permission. Console execution is disabled because placement needs a character.

```text
.hotspot list [map zone page [name or ID]]
.hotspot new circle Name
.hotspot new zone Name
.hotspot new polygon Name
.hotspot edit ID
.hotspot name New name
.hotspot center [worldX worldY]
.hotspot radius yards
.hotspot point [worldX worldY]
.hotspot undo
.hotspot clear
.hotspot settings bonusPercent durationSeconds weight cooldownSeconds showIcon showHighlight
.hotspot preview
.hotspot save
.hotspot enable ID
.hotspot disable ID
.hotspot next ID
.hotspot stop ID
.hotspot announce 0
.hotspot announce 1
.hotspot cancel
```

Use map `4294967295` and zone `0` to list the entire catalog. Settings booleans are 0/1. Supported
limits are 0–1000% bonus, 60–86400 seconds duration, weight 0–1000, cooldown up to seven days, circles
of 1–5000 yards and simple polygons of 3–64 vertices. A zero weight excludes weighted rotation;
an explicit GM "next" request can still select that location, subject to map and geometry restrictions.

## Data provenance and rebuilding

The seed is an independent reconstruction, not recovered Ascension server scheduling code. It
retains all **2,715 quest-objective records** from the community data export and **414 whole-zone
records** after reconciliation with the current client. Of 3,129 records, 2,665 are initially enabled;
154 lack usable map geometry. Other disabled records include instance/unsupported-map locations.
Realm configuration further restricts eligibility at runtime.

- Public source: [hertigservices/ascension-data export](https://github.com/hertigservices/ascension-data/tree/main/supplemental/exiles-db-export),
  reviewed revision `3b42472af59df17c6bf05e4d2a4c4f5f2b089fe0`.
- Archive: `exiles-db-export-2026-09-13/coa-public-2026-09-13-tables.tar`, SHA-256
  `0f69f4bd01b9f787b4a4bb267fba6711b905300aeaf4f42873bbd9f5446d8549`.
- Effective WorldMapArea.dbc SHA-256:
  `203c101fcf1fc33e27dcde8ba721db402814567d3976825f0f94012188aaed09`.
- Effective AreaTable.dbc SHA-256:
  `696a8fa876b55503217827d92ddb48c57078ab9cfde8af3c46023a1f7b15c8ec`.

Archived percentages are converted using the archived map bounds they refer to. Current client
bounds govern whole-zone previews. A recovered zero-radius single point receives an explicit
150-yard default; this is a new policy, not a recovered original radius. Original objective records
and radius provenance remain in each database payload.

```text
python tools/import_hotspot_locations.py --archive tables.tar --client-maps WorldMapArea.dbc --areas AreaTable.dbc --output staging
```

The checked-in migration needs no archive download at installation. Regeneration checks the archive
hash and objective count and produces location JSON, migration SQL and provenance hashes.

### Boundary refinement

The initial export contains spawn centroids/radii, not quest polygons. Its Barrens coverage was only
two objective rows. The refinement uses the effective server's `quest_poi` and `quest_poi_points`,
the same boundary data requested by the client's quest overlay (`WorldMapBlobFrame:DrawQuestBlob`).
Only outdoor, floor-zero, non-completion objective polygons with valid simple boundaries are imported.
Single-point icons, malformed/self-crossing boundaries and unsupported maps are not fabricated into polygons.

The refined default catalog has 6,164 visible entries: 5,009 distinct polygons, 741 remaining approximate
spawn circles, and 414 whole-zone entries, including disabled/unresolved records. The Barrens has 106
entries including its whole zone. 1,513 old circles are retired in favor of objective polygons; 2,661 exact
boundary duplicates are retired; 290 holiday-derived seed entries are retired. Holiday/event quest relations
and explicit seasonal quest sorts also exclude holiday polygons from import. All original records remain
recoverable. Names use the covered zone and a geographic sector rather than the original target name;
they do not claim an unverified named subzone. GM tooltips label approximate circles.

Exact polygon equivalence ignores starting vertex and winding direction. Nonidentical overlaps are left
for deliberate GM selection, avoiding transitive overlap chains that could cover most of a zone.
Refinement updates only unchanged revision-1 seed names; GM edits are preserved, including any such
edited entry requiring manual holiday/duplicate review. Reapplication preserves new edits and schedules.

The reviewed upstream revision is `4b25cd5d56d353061075916ee6c6f071b57d0465`; the qualification base
remains `231b9958f1a33cc256078fcdcb635c36b29b3580`. The intervening Architect board change does not
alter hotspot or quest geometry handling. Upstream base SQL contains 18,771 POIs and 57,162 points;
the effective Area 52 database contains 24,941 POIs and 90,131 points after applied updates. The latter
is the refinement source, not a claim that untouched upstream base SQL is identical. This remains an
independent reconstruction, not recovered official Ascension scheduling logic.

Reproduce refinement with tab-separated, headerless exports ordered by their primary keys:

| File | Columns / query source | SHA-256 of reviewed export |
| --- | --- | --- |
| poi.tsv | QuestID,id,ObjectiveIndex,MapID,WorldMapAreaId,Floor from quest_poi | d07a4ad6bb9837e6e7ecc33aff56b03b6294195b6a5af0aba4d2a5f2d25422a2 |
| points.tsv | QuestID,Idx1,Idx2,X,Y from quest_poi_points | 5790f7a026eaaebe01d659d843ab9ce1d241bdc1c90846f92c86725e91efd315 |
| holidays.tsv | Seasonal quest IDs, union creature/gameobject event quest IDs linked to a nonzero game_event.holiday | e0c3a17ffe13de6c93cbe5709f1feb40747f59681ee470cd04392d22470cd937 |
| quests.tsv | ID,QuestSortID,HEX(LogTitle) from quest_template | 236574526c337e706577ea6a8560e2fd12c163e35254575d7d3276c259aa5e70 |

```text
python tools/refine_hotspot_locations.py --seed staging/locations.json --maps WorldMapArea.dbc --export exports --output refined
```

The committed second migration includes the resulting boundaries; deployment needs no external data service.
The client and server must be updated together for compound geometry and direct-map editing.

## Qualification

GM area selection uses a gold perimeter for circles, polygons and merged areas. Whole-zone selections
use a padded border around the XP icon instead. SHIFT-click either map markers or browser rows to
toggle merge candidates. Player hover coverage remains separate from GM selection, including when
the player-facing highlight is disabled. World-map markers use the client's native world-to-map
projection and clear old boundary overlays when the viewed map changes.

The manager includes Teleport to center and Delete selected. Teleport preserves the GM recall point,
requires valid terrain and an allowed outdoor map, and rejects combat, taxi flight, vehicle use,
death and an existing teleport. A hotspot center can lie outside a concave or merged boundary.
Permanent deletion requires an explicit irreversible-action confirmation and a current revision.
The server removes the saved location and active/queued state, invalidates related edit/merge drafts,
and retains an ID allocation floor so a deleted ID cannot be reused. Existing periodic XP checks
remove a deleted location's bonus. General settings and other hotspots are unaffected.

Browse / active occupies the full bottom button row. These changes require matching native client
UI and server updates; a source push does not activate them or publish launcher assets.

The October 12 manager update reviewed upstream `main` at
`b8e4f26165334d77264e0ab041c804f16a5976c9`. Its changes since this package's pinned base
`231b9958f1a33cc256078fcdcb635c36b29b3580` do not change the hotspot module, world database
interfaces or map interfaces used here. Keep the standalone PR on hold for in-game acceptance.

Install Lua test dependencies with `python -m pip install -r client/requirements-test.txt`. Run checks
through the repository verifier:

```text
python -B tools/verify_all.py --base BASE --stages source,build,unit,harness --harness xp_hotspots --harness test_hotspots --harness test_hotspot_data
python apps/codestyle/codestyle-cpp.py --files src/server/coa/XPHotspots.cpp src/server/coa/XPHotspotRules.h
python apps/codestyle/codestyle-sql.py --files data/sql/updates/pending_db_world/rev_20261011_00_hotspots.sql
```

Before the upstream PR, run the required full verifier against the pinned upstream base and reconcile
any failures with that base. Unit geometry and mocked Lua results do not establish rendered-client
or multiplayer acceptance. The in-game checklist is in [xp-hotspots-acceptance.md](xp-hotspots-acceptance.md).

The optional `test_hotspot_database` harness requires an empty disposable MySQL 8 database named
`hotspot_test`, reachable at `127.0.0.1:3306` as root without a password inside an isolated test
container. Set `XPHOT_DISPOSABLE_MYSQL=1` only in that fixture and select it through
`tools/verify_all.py --stages harness --harness test_hotspot_database`. It imports the migration,
checks every JSON payload, edits a record and the announcement setting, then reapplies the migration
to verify that existing state survives. It refuses a nonempty database and never uses realm credentials.

Continent and world overview markers use the client map projection, including expansion-map starting
islands such as Bloodmyst on Kalimdor. Locations and General Settings share the same panel dimensions
and continuous input borders. Delete confirmations appear above the map and restore their original
frame layer when dismissed.

Overview markers are filtered by geographic continent before projection. Azeroth excludes actual
Outland locations; expansion-map blood elf and draenei starting zones remain on Eastern Kingdoms
and Kalimdor respectively. Outland and Northrend overviews show only their own locations.


## Polygon painting and permissions

Level 2 GMs have all hotspot controls except permanent deletion, including creation, merging,
renaming, boundary painting, saving new drafts, and all operational/general settings.
Permanent deletion requires Level 3. Both addon requests and text commands enforce this server-side;
the client disables Delete selected for Level 2 while enabling all design tools.

Open Drawing tools while creating or editing a polygon. Points retains the original vertex editing.
Trace perimeter uses CTRL-drag, then closes the outline on release. It replaces the draft boundary
atomically after validation; crossing or invalid outlines leave the previous draft intact. A trace
is limited to 60 sampled points and reports when it reaches that limit.
Brush / fill uses CTRL-drag to add a round or square footprint, with radius 5�1000 yards and a live
cursor outline. Segments bridge consecutive sampled positions, preserving concavities instead of
filling the whole convex hull of the hotspot. Begin on or overlapping the existing polygon;
disconnected areas and holes are rejected. The resulting polygon is limited to 256 vertices.
Complex boundaries may be simplified by at most two yards only when this retains all prior coverage;
otherwise the edit is rejected. Undo stroke restores the previous boundary (one level).

Saved circles and polygons can switch shape through Edit selected and Circle/Polygon without changing
the hotspot ID or operational settings. Polygon-to-circle chooses an enclosing radius, which the GM
can adjust. Save Draft remains explicit and waits for drawing acknowledgement. Client previews and
server mutations are drafts until saved; a source push does not install this client/server pair.

The painting/permission follow-up reviewed upstream main at `d5ff1cfa46e4bb0ae98b90a289cec2a7c237a13a` (October 12). The standalone
upstream PR remains held for in-game acceptance and full qualification. Test cursor scaling, drawing
across existing map icons, cancellation on map changes, invalid outlines, Undo, and Level 2/3 accounts
in the actual client before acceptance.
