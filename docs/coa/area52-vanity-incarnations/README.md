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
model asset in the installed client. Two additional entries use spell-based
humanoid visuals, whose visual kits and effect files are present. All 43 ammunition entries have existing
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

## Spectral forms resolved

Further investigation establishes that appearances 47636 and 47647 are spell
appearances, not missing creature models. Their negative display references point
to 499397 (Moonkin Astral Form) and 499396 (Tree of Life Spectral Form). The client
collection artwork depicts humanoid characters with blue astral or green spectral
effects respectively. Both spells apply a Dummy aura and have no effective SQL
override or bound script in the reviewed Area 52 database.

The repair retains the player's native model while the corresponding learned form
is active and applies its recovered visual aura. Visuals are removed on form exit,
selection changes, visibility changes, death, or transform/clone suppression. Form
benefits remain controlled by the original form spell. Ownership and category
checks still apply, and no new form spells are taught.

| Appearance | Spell | Visual / kit | Installed effect |
| --- | --- | --- | --- |
| 47636 | 499397 | 21088 / 21062 | `Spells/druid_glyphofstars_base.m2` in patch-N |
| 47647 | 499396 | 21232 / 21206 | `Spells/custom_leaf_form.m2` in patch-N |

The [Ascension spell listing](https://db.ascension.gg/?spell=499397) independently
identifies Moonkin Astral Form as a Dummy aura. The actual visual chain and intended
humanoid presentation come from installed client data and collection artwork.
Native regressions and the Area 52 build pass; appearance quality still needs
in-game acceptance. No client patch is needed for these two effects.

Upstream was fetched again for this follow-up at `7bcaec8a6`; its only new commit
since the original pin is an unrelated Witch Doctor fix, so the focused branch
keeps its original tested base.

## Unresolved data

| Appearance | Item | Reason |
| --- | --- | --- |
| 47640 | 97742, Oakheart | Display 950002 points to `creature/vsancient/vsnightmareancient.mdx`; neither that file nor its `.m2` variant was found in the installed client. |

Oakheart still needs the correct shared client asset; its valid DBC row alone
does not prove that it renders. It is not reported as functional.
The missing Oakheart asset was also absent from 35 historical/base client archives.
The community cache corroborates display 950002 for incarnation creature 80963;
the separate Oakheart creature 80648 uses display 68127, which points to the same
missing model family. Other installed ancient models are not evidence of the
intended Oakheart model and are not substituted.
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
The targeted C++ style scan also reports two existing same-line braces in the
unmodified GM vanity-unlock methods; the new incarnation code introduces no
reported style violation.

This is structural/native verification. In-game visual, relog and pet-resummon
acceptance remains required before claiming full gameplay functionality. No
CERTIFIED or VERIFIED gameplay labels are changed. The server repair is staged;
no restart, client publication or activation is performed by this source milestone.
