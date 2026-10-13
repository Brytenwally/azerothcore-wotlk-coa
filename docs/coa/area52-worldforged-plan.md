# Area 52 acquisition restoration plan

The target is broad coverage of static acquisitions and reconstruction of the interaction-based
routes, not a single-cache demonstration. Missing coordinates are research tasks; they are not a
reason to abandon a supported acquisition. Work continues on independent entries while gaps remain.

## Inventory and batches

The [worklist](area52-worldforged-worklist.md) assigns all 176 references an initial research track:
45 static-object candidates, 24 vendor routes, 59 creature/drop routes, 45 interaction routes,
and 3 catalog-identity questions. These are provisional, mutually exclusive work assignments;
many enchants have more than one acquisition method. They do not measure completion.

1. Resolve cache lock/key behavior and scroll rewards before activating static placements.
2. Batch dungeon caches by instance: Ragefire, Shadowfang, Scarlet Monastery, Blackfathom,
   then Stockades, Wailing Caverns, Razorfen, Dire Maul, Scholomance, Stratholme and Blackrock.
3. Batch outdoor objects by zone, including caves, towers and vertical placements.
4. Restore vendor inventory and confirmed creature loot independently of static cache alternatives.
5. Reconstruct interaction routes in parallel with the remaining location research, starting with
   ordinary collection quests and progressing to timed objects, sequences and custom events.

For each entry record realm/season, enchant and scroll IDs, alternate methods, map/XYZ/orientation,
object/NPC identity, phase and access requirements, lock and consumption behavior, respawn/claim
rules, source links, video timestamp where available, and tests still required.

## Evidence and coordinate workflow

Use the effective Area 52 client catalog as the eligibility check. Match Area 52 references against
upstream CoA records and mode-attributed player caches. Videos from other modes may establish a
landmark or shared prop, but cannot alone establish Area 52 eligibility or acquisition rules.

Locate the object in footage using the minimap, dungeon room, nearby creatures and landmarks.
Translate map positions using the effective Ascension map bounds. Resolve floor height from terrain
or geometry; nearest-object heights remain test proposals, especially in multi-level buildings.
If the exact original point is unrecoverable but the acquisition location is supported, prepare a
clearly labelled reconstructed placement in that location for testing instead of blocking the batch.

YouTube research has started; no video has yet been watched or timestamp-qualified in this package.
Search results and video titles must not be recorded as viewed footage. Capture a video URL and
relevant timestamps when inspected, and distinguish observable interactions from narrator claims.

## Initial interaction research

### Dark Ranger

The [Ascension quest record](https://db.ascension.gg/?quest=25003) documents the collection quest
and Cache of the Dark Ranger reward. Quest 25003 is absent from the currently queried Area 52 world.
Recover the quest starter/ender, four item identities, appropriate creature loot and cache reward;
then build on ordinary quest infrastructure. References disagree about the bow/arrows source pairing:
resolve this using quest data, item records and footage rather than swapping them by guesswork.

### Arcane Thunder Seer

Research four timed books, their locations, the combining item and consumption/expiry behavior.
The Area 52 reference reports a missing or moved Dalaran book, so check footage by date and realm.
Test missing components, timer expiration, duplicate collection, bag capacity and successful combine.

### Dark Apotheosis and Mechanical Arm

Trace the existing prerequisite chains first: Dreadsteed/Mor'zul and Sputtervalve/Ziz Fizziks.
Reuse existing steps where compatible with HERO progression. Custom quest rewards must not bypass
the standing restriction against learning unrelated class abilities through quests.

### Legacy of the Arcanist

Research the staff pickup, NPC launch interactions, four distinct elemental sphere credits and final
reward. Model progress per player; one player's sequence must not consume or complete another's.

## Cache contract: owner-confirmed acquisition rules

The official [Season 8 Chapter 2 announcement](https://ascension.gg/es/news/season-8-chapter-2-overview/398)
describes five fragments forming a key for a chosen Worldforged cache, alongside expansion to Area 52.
The owner confirmed the intended Area 52 contract on October 12, 2026:

- Boss scroll drops remain an acquisition route.
- Each boss with a scroll drop also has a cache offering the same scroll as an alternative route.
- Combining five Worldforged Key Fragments creates one Worldforged Key.
- Opening a cache consumes one key and awards that cache's mapped scroll.
- Worldforged scrolls bind on pickup and cannot be auctioned. The owner confirmed repeat cache
  collection is allowed with another key, even when the enchant is already collected.

The live world contains Worldforged Key 97729 and Fragment 97730. Inspect their existing combine
spell and Lock.dbc before implementing the contract. Do not inherit the gear pickup module's
permanent once-per-character claim restriction. Keyed acquisitions need their own handling.
Do not extend the boss-cache rule to bypass unrelated quest or event prerequisites.
Flameburst's original `worldforged_pickup` prototype was removed before activation. The new
16-cache batch requires a dedicated keyed handler and must not be activated until that is implemented.
Key consumption and reward delivery must handle full bags, concurrent users, retries and failures
without losing a key or duplicating a reward. Fragment drop sources/rates are a separate research task.

An [official changelog](https://ascension.gg/fr/changelog/1?page=115) independently confirms caches
near the sources of Flameburst and Ragefire Cleave and an off-path Earthborer Acid cache in its instance.
It also records moving Fiery Heroic Leap out of lava; older footage may therefore show obsolete positions.

## Qualification and delivery

Test SQL idempotency and foreign identifiers, reward mappings, full bags, simultaneous users,
repeated attempts, key consumption, reconnect persistence, and instance reset where applicable.
Interaction routes additionally need prerequisites, failure/reset behavior and multiplayer isolation.
Use fixture/structural checks before staging; mark geometry and gameplay checks separately.
Publish focused source milestones to the fork. Activation, tester release and upstream PR are separate.
