# Area 52 Prestige integration

This directory contains only the Prestige integration for Area 52's existing backend. It does not
import that backend into Dawnrise or enable Area 52 on upstream main. The ordinary upstream build
uses its own realm reset implementations unless an Area 52 handler registers.

`prestige.patch` targets the Area 52 source baseline identified by `baseline.json`. It includes the
shared interface, registration, module dispatch, realm-cap resolution, specialization-lock guard,
and the two reset implementations kept here for focused native tests. Apply it only to that matching
Area 52 tree; do not apply it to upstream's differently named and structured Free Pick backend.
The manifest hashes normalize line endings to LF. A baseline mismatch requires reconciliation.

The reset validates both active specializations before mutation, then checks the same identities
again at execution. It rejects missing build unlock spells, invalid indices, unavailable or busy
state, and ordinary unsafe player states. Build and enchant selections are independent: Area 52
stores its build index in `core.area52.spec.active`, while its current enchant backend uses
`GetActiveSpec()` and native `GetSpecsCount()` unlocks. No new enchant selector is introduced here.

Only the active build's selections, primary-stat selection within those entries, saved ranks,
saved action bars and active archetype association are reset. Automatic archetype learning is paused
and its pending state invalidated. Only the active enchant specialization's equipped slots are
cleared and synchronized. Other build slots, other enchant slots, unlocks and the account collection
are preserved. The existing Prestige level change restores ordinary level-1 grants; existing native
selection removal and rank authorization remain in force.

The helpers stage settings in memory for the existing Prestige save. Empty-selection synchronization
also respects that deferred save. This does not redesign the pre-existing Prestige database failure
recovery or provide atomicity across unrelated database operations in the original module.

Run `tools/verify_all.py --stages harness --harness prestige_area52` for the native reset matrix.
Providing `AREA52_PRESTIGE_BASELINE` additionally checks exact patch application and reversal on a
temporary copy of the matching baseline. The matrix executes the shipped reset bodies with engine
adapters: it establishes mutation boundaries and rejection behavior, not live gameplay or UI behavior.
The Area 52 integration also requires its native build/unit checks before deployment. Neither a
passing upstream build nor this source patch activates the realm or distributes the client UI change.
