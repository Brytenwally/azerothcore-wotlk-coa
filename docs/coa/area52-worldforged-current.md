# Worldforged acquisition implementation status

This status supersedes earlier placement-only milestone notes. Source is staged for Area 52;
the live server, tester release and main PR have not been changed.

## Implemented

- 33 source-to-scroll loot mappings, using the current base and difficulty-template loot IDs.
  30 have the owner's 50% base chance. Shadow Word: Gore, Lunar Aspiration and Megafrost from
  Kashoch use the [documented 100% chances](https://ascension.gg/fr/changelog/1?page=115).
  Core loot-rate configuration can still modify ordinary base chances.
- 22 normal-mode reconstructed cache placements and dedicated scripted object templates.
  The latest additions are Arcanist's Explosion, Fire And Fury, Serpent Form, Consumed By Shadows,
  Void Zone and Megafrost. All sources, scroll IDs and coordinates are in the placement manifest.
- One Worldforged Key buys one bound scroll. No permanent claim ledger or already-collected gate.
  Five fragments create one bound key. Insufficient ingredients, invalid user/context, failed storage
  or full bags leave the cost untouched. Successful exchanges save cost and reward in one inventory
  database transaction. Repeated requests each require and consume another key.
- Worldforged-only bind-on-pickup migrations cover 1,165 eligible client item IDs and bind existing
  item instances while preserving their other flags. The preparatory read found 68 unbound instances.
  Ordinary non-Worldforged scrolls are not targeted.
- Great Cat Spirit's generic loot is copied into its own loot ID before adding Lunar Aspiration.
  The shared original has 23,327 creature-template users; it is not modified.

## Deployment

Use the new handler and script-loader registration with migrations 07 through 10. Set
`Area52.WorldforgedAcquisition.Enable = 1` only on Area 52 after installing the matching package.
Default is disabled. The original Flameburst-only migration was removed before activation.
Do not import the historical once-per-character gear pickup handling for these objects.
Apply the character binding migration during the controlled offline update.

Compilation used the existing reconstruction verification build, not a pristine latest-main PR
qualification build. A second incremental build against the current Area 52 source also passed. Activation still needs
in-game checks of height, collision, accessibility, visuals and actual key/scroll behavior.
The positions are proposals beside observed source spawns, not recovered original cache coordinates.

## Verification

- C++ and SQL codestyle passed.
- `VERIFY ALL: PASSED`: source checks, compiled server, placement and loot fixture harnesses,
  and a native harness compiling the actual handler against mocked inventory/database collaborators.
- An isolated MySQL fixture copied relevant live world definitions into a disposable database and
  applied all four migrations twice. It verified the 33 source drops, 22 spawns and binding changes.
  The fixture was deleted afterward; no live world or character rows were changed.
- The native test covers missing keys, insufficient fragments, full bags, storage failure,
  disabled configuration, distance, phase, wrong class, repeated purchases and different players.
  It does not simulate network concurrency, database outage or full in-game combat.

Initial failures were tooling/setup issues: a Windows temporary-directory cleanup denial, an
unmounted build cache, and CMake not discovering a newly added source file until reconfigured.
Corrected runs passed. No claim of in-game certification or complete upstream qualification is made.

## Remaining acquisition gaps

The initial Area 52 guide contains 176 named references; this package does not claim to finish all
of them. The full client contains many more Worldforged variants, including ability/creature-family
drops. No default rate for ordinary trash or ability-conditioned drops was authorized or invented.

- Creature-family/ability-specific scrolls need their triggering spell, target-family and eligibility
  rules plus rates. Boss 50% defaults are not applied to every creature in a family.
- Shady Dealer routes: no Shady Dealer templates were found in the current world. Restore vendor
  identities, faction access, locations and prices from attributable data before adding inventory.
- Static landmark objects still need resolved positions, floors or scripted-spawn anchors.
  Water Geyser and Firefall have cross-mode cache pins to compare with Area 52 geometry. Their
  boss drops are already included. Uncertain coordinates do not block the independent completed entries.
- Dark Ranger: recovered quest 25003 is absent; items 400005–400008 and reward cache 1414014 exist,
  but the reward cache has no item loot and no matching quest-start object was found. The
  [quest record](https://db.ascension.gg/?quest=25003) resolves bow versus arrows attribution:
  Scarlet sources for the bow, Andorhal victims for arrows. Quest object identity/placement,
  item drop rules and reward contents still need reconstruction.
- Arcane Thunder Seer and Legacy of the Arcanist need timed/per-player event reconstruction.
- Rotting Breath: existing quest 3525 rewards Dragonclaw Ring. The Area 52 guide names the escort
  route and a changelog documents guaranteed acquisition, but the event-boss versus quest-reward
  boundary is unresolved. Do not add a duplicate reward to the quest by assumption.
- The truncated Nesingwary reference is resolved to item 132420, spell 982375. Hoplite and Dragon
  Warrior exist in the client scroll catalog but did not pass its Worldforged flag filter; preserve
  their current mode definitions pending reconciliation with the historical acquisition guides.
- Fragment drop rates/sources and visual acceptance of cache placements remain to be qualified.

These gaps are retained for further research; the implementation does not replace their requirements
with free scroll pickups or silently mark unimplemented routes complete.
