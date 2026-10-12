# Vanity incarnation audit and repair

Reviewed upstream `jealous-sound/azerothcore-wotlk-coa` at
`c1ce0f504888008db323ad99db50077e2f77b199` on 2026-10-12, the effective
Area 52 collection DBCs, installed primary client resources, and the
mode-attributed hertigservices community creature cache. These are client
definitions and an independent server reconstruction, not Ascension backend
source or evidence of live gameplay certification.

## Scope and results

The complete appearance catalog contains 320 incarnation entries: 269 shared
player-form/demon appearances, 43 ammunition appearances, and eight CoA-specific
appearances. The catalog is larger than the earlier name-based scan of 59
creatures. All 321 referenced source items exist in the effective world database;
all 320 appearances have source-item appearance mappings.

| Category | Entries |
| --- | ---: |
| Bear / Cat / Travel / Aquatic | 46 / 64 / 13 / 2 |
| Flight / Moonkin / Tree / Ghost Wolf | 7 / 24 / 30 / 9 |
| Imp / Voidwalker / Succubus / Felhunter / Felguard | 21 / 6 / 6 / 11 / 17 |
| Metamorphosis | 13 |
| Ammunition | 43 |
| CoA Knight / Draconic / Lich / Shadowhound | 1 / 2 / 1 / 4 |

267 shared model entries resolve to a valid display; 266 also have a located
model asset in the installed client. All 43 ammunition entries have existing
projectile mappings. Eight class-specific entries are outside Area 52's approved
class progression; this change does not activate their CoA mechanics.

## Repair

Saved selections previously had no general connection to player-form or demon
model application. The repair connects the existing account appearance ownership,
per-character selection, visibility, login synchronization and display-change
paths. It handles ordinary and dire bear, normal and swift flight, the remaining
listed forms, and the five permanent summoned demon families. Pet reconciliation
uses the existing cosmetic timer; no new timer or database polling is introduced.

Selections must be owned, belong to the requested category, and resolve to a
valid display. Temporary transform/clone auras and unrelated scripted models
take precedence. Removing a selection restores the normal form/pet model. Native
pet identity, spell lists and combat stats are not rewritten. This does not grant
the underlying form or summon spell.

The authoritative AppearanceCategories IDs are 17–32 for shared incarnations;
similarly named Lua transmog enum values are not packet category IDs.

The explicit positive display on appearance 47651 (Tree of Wrath, Solar) supplies
the missing creature-map reference. Conflicting appearance metadata for Slatebound
Incubus and Autumnal Tree of Life uses the creature display mapping, corroborated
by the community Free Pick cache (99203 and 40971 respectively).

## Unresolved data

| Appearance | Item | Reason |
| --- | --- | --- |
| 47636 | 499402, Moonkin Astral Form | Creature 499397 is absent; negative display reference cannot establish the intended model/effect. |
| 47647 | 499401, Tree of Life Forest Guardian Form | Creature 499396 is absent; no recovered model reference. |
| 47640 | 97742, Oakheart | Display 950002 points to `creature/vsancient/vsnightmareancient.mdx`; neither that file nor its `.m2` variant was found in the installed client. |

The first two invalid selections are rejected rather than displaying a fallback
pig. Oakheart still needs the correct shared client asset; its valid DBC row alone
does not prove that it renders. These entries are not reported as functional.
The missing Oakheart asset was also absent from 35 historical/base client archives.
79 source items lack a separate VanityCollection row, but all have the existing
item-to-appearance collection path. Appearance ownership and a deliverable vanity
item are distinct; this change does not invent additional reclaimable item records.

## Upstream references

- [Creature preview mappings, #353](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/353)
- [Display data streaming, #5794](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/5794)
- [Shadowhound incarnation implementation, #6820](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/6820)
- [Community cache](https://github.com/hertigservices/ascension-data)

The Shadowhound implementation is used as a reference for model resolution;
Witch Hunter mechanics are not imported into Area 52. Reincarnation resurrection
work is unrelated to vanity transformations.

## Verification and release state

Native extracted-method regressions cover all eleven supported form IDs and ten
demon entry aliases, selection removal, hidden appearances, missing ownership,
category isolation, transformation priority, pet replacement, owner isolation,
reentrant display hooks and out-of-world guards. The existing Woodworking
collection harness also passes with its mocks updated for current upstream
interfaces. The Area 52 integration compiles against the retained deployed source
and passes the incarnation harness.

The broad Windows source suite passed 19 checks and failed four: the comment
tool's symlink test lacks Windows privilege, and the verifier, SQL-capture and
gameplay-batch tool tests fail in their existing test infrastructure. Those tools
and tests are unchanged by this repair. The first sandboxed attempt additionally
hit temporary-directory permission errors; the host rerun removed those errors.
Do not describe this result as a completely passing upstream qualification. A
clean Linux baseline comparison is still needed before an upstream PR.

This is structural/native verification. In-game visual, relog and pet-resummon
acceptance remains required before claiming full gameplay functionality. No
CERTIFIED or VERIFIED gameplay labels are changed. The server repair is staged;
no restart, client publication or activation is performed by this source milestone.
