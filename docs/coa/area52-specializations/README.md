# Area 52 specialization integration

This focused integration patch targets the existing Area 52 Free Pick selection, Active Build and
Advancement services. Those dependencies differ from upstream main. It is not a standalone upstream
feature PR and must not replace upstream's `AscensionFreepick` implementation.

The original client's twenty specialization slots use their existing swap spells, unlock tomes,
names, icons, equipment associations and Mystic Enchant preset associations. Slot one is taught on
login; additional slots still require their original unlock spell. No prices or distributions change.
Only Area 52 Free Pick Heroes use the new handler.

Each slot retains its Advancement entries (including Primary Stat), trained ranks, full action bars,
Active Build ID/revision and Auto-Learn setting. Existing characters retain their current selection
as slot one. The current selection remains authoritative at login; the integration does not restore
an outdated slot snapshot over a player's subsequent edits. Existing `core.*` character settings
provide persistence without a new character table.

Switches validate ownership and the destination selection before applying. Combat, battlegrounds,
arenas, death, movement, teleport and pending Active Build operations refuse switching. A saved
selection that no longer satisfies level or budget rules is retained and refused, not truncated.
Trained ranks above the current level are not regranted. The normal selection pipeline handles
removed buffs, linked abilities and equipped ME grants. Pets and controlled summons are cleared.

Pending Active Build callbacks are invalidated by generation, queued requests are cleared, and
the new slot's plan is loaded independently. The client receives the full twenty-slot state and
the changed slot before resuming its native equipment/ME-preset behavior. Action buttons whose
spells arrive with a delayed preset grant have a ten-second, event-driven restoration window;
occupied buttons are preserved and each pending button is consumed once. No polling is added.

The server SQL binds the twenty original spells to `spell_area52_specialization_swap`. It preserves
other realm handlers and original client spell definitions. No addon, client archive or launcher
change is required for this integration. The separate talent-certification addon repair is not
included or released here.

## Provenance

- Upstream reviewed: `jealous-sound/azerothcore-wotlk-coa` main
  `b366fa8434d0a50a03c6d4cf0611eae37d99821e`.
- Upstream `AscensionFreepick.cpp` supplies the native swap-handler and independent-build pattern;
  `AscensionWildcard.h` and the installed client agree on the twenty swap spells.
- Installed Area 52 `SpecializationUtil.lua` SHA-256:
  `ac738fc25da4cc04e0bf82a2d8234095442e897c363109391e0bee64e2a97922`.
- The live world database contained the specialization tomes, but no Free Pick swap-handler bindings.
- `source-files.json` records the reviewed Area 52 baseline and candidate hashes. Apply the patch to
  that integration baseline, using `git apply --check --ignore-space-change specializations.patch`
  first to accommodate Windows checkout line endings. Do not apply it blindly to upstream main.

## Verification and activation

The pre-integration implementation fails the new slot regression. Candidate verification runs through
`python -B tools/verify_all.py`: worldserver build plus `area52_specializations`, `area52_free_resets`,
`freepick_advancement` and `area52_build_protocol`. The specialization harness compiles production
save/switch/restore methods against fixture Player dependencies; it is not an independent client or
live reconnect test. It covers repeated slot round trips, serialized-settings restoration, all
144 action buttons, unrelated-spell preservation, locked/invalid slots, unsafe-state refusals,
Active Build separation, delayed grants, occupied buttons and expiration. The remaining harnesses
cover selection budgets, prerequisites, rank selection, Primary Stat, resets and build packets.

Final result: **VERIFY ALL: PASSED** (build and four harnesses), recorded in
`outputs/Area52_Specializations/preset-checks/report.json` in the working environment. C++ and SQL
changed-file style checks passed. The source-package checks passed in
`outputs/Area52_Specializations/publication-checks-final/report.json`. Unit and gameplay stages were
not run; no live reconnect or combat result is claimed. During development, a fixture syntax error
and the reset fixture's old three-argument call were corrected; the production build passed.
The initial package source check lacked the sparse checkout's mechanics catalog; restoring that
dependency resolved it. No tester corrections were needed for these checks; gameplay behavior and
runtime flakiness remain unmeasured.
Live acceptance remains separate: switch between two unlocked slots, relog, confirm point budgets,
bars, linked equipment/ME presets, and independent Auto-Learn behavior. Source publication does not
apply the SQL, replace the running worldserver, or release client assets. Activate with the standard
three-minute announcement and restart only when requested.
