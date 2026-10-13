# First placement construction batch

Sixteen cache templates and spawns are prepared in
`rev_20261012_07_area52_worldforged_cache_placements.sql`.
The [manifest](area52-worldforged-placement-batch.json) includes reward item/spell mappings,
source links, existing creature anchors, reconstructed positions and remaining test status.

| Location | Scrolls |
| --- | --- |
| Ragefire Chasm | Flameburst, Ragefire Cleave, Ragefire Nova |
| Shadowfang Keep | Piercing Cleaver |
| Scarlet Monastery | Herod's Legacy, Interrogator's Immolation, Veil of Shadows |
| Blackfathom Deeps | Hydra's Venom, Twilight Shock |
| Stockades | Knifestorm |
| Razorfen Downs | Shadow Word: Gore |
| Blackrock Depths | Shield of the Emperor |
| Scholomance | Shadow Dagger, Bone Spear |
| Maraudon | Razorlash |
| Moonglade | Lunar Aspiration |

These are reconstructed test positions three yards beside the current normal-mode source creature,
using its floor height. They are not exact original Ascension cache coordinates. Where a guide names
a specific statue, hut or pillar, refine the test position after a geometry/footage survey.
The owner confirmed the boss-drop/cache alternative, so named boss-drop references can support
a reconstructed cache even when they do not explicitly name a cache location.

Only normal difficulty is included. Several current heroic/mythic anchors differ; these must not be
blindly copied into other instance versions. All current phase masks are 1.

## Activation prerequisites

- Implement and validate `area52_worldforged_scroll_cache`. The template is a scripted goober,
  not a lootable gear pickup. No implementation of this new handler is shipped in this milestone.
- Enforce Worldforged scroll bind-on-pickup, including these mapped reward items. The manifest
  records the requirement; this placement SQL does not alter item templates or existing inventories.
- Exchange one key for one scroll without a permanent character claim restriction. Repeated
  purchases remain possible. Preserve the boss drop as a separate route.
- Validate five-fragment key creation and full-bag/failure behavior.
- Test floor, reach, nearby combat/path obstruction and multiplayer interaction in game.

Do not deploy this SQL alone. The earlier single Flameburst SQL was never activated and has been
removed from this branch; existing upstream cache templates and per-character gear claims are unchanged.

## Checks

Source checks and the focused placement harness passed through `tools/verify_all.py`.
SQL codestyle passed. The harness checks unique identities, anchor/map consistency, normal-mode
scope, repeat application, and collision preservation. It executes SQL using SQLite with the
MySQL no-op upsert translated to SQLite's equivalent; it is not a MySQL integration or gameplay test.
World database reads found the reserved template/spawn ranges vacant at preparation time.

## Continuing research

Next candidates include Earthborer Acid's bridge position; Silverlaine hall/stage/upstairs caches;
Mutanus, Tuten'kash and Balnazzar whose base creature entries did not produce static anchors in the
initial query; outdoor landmark caches; and Maggot Trap's supply-crate loot rather than a guessed
unique chest. Trace scripted/difficulty spawns where base entries are absent. Quest and timed-event
routes remain in the full worklist and do not block independent placements.
