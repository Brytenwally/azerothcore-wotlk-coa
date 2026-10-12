# Transformation model repair

Flame Ascendance (760000) and Air Ascendance (954831) referenced absent creature templates 173524 and 173525. Area 52 runtime logs confirmed the transform handler selected its pink-pig fallback, display 16358. The installed client Creature.dbc explicitly maps these entries to displays 42853 and 151196. Water Ascendant 173523 maps to 42854 and had the same structural gap.

The pending migration supplies 218 missing creature/model mappings covering 292 transformation effects, including alternate spell IDs. It uses explicit client creature entries, preserves every distinct recorded display variant, and checks the complete display-to-model chain and model-file availability. It does not derive display IDs from creature IDs. All proposed display-to-model IDs match the server DBC. No client patch is required for these records in the reviewed client.

The migration only fills entries with no model rows and no world spawns. Existing templates and existing model mappings remain unchanged; newly created templates are visual references with no new spawns, scripts, loot or services. Reapplication does not duplicate or overwrite mappings. It intentionally leaves partly populated existing mappings alone.

## Audit scope and exceptions

The census examined 1,836 transform effects in the union of installed client and server Spell.dbc, with SQL spell rows taking precedence and recovered native effective exports used where available. 1,466 already have valid model links; 292 receive repairs. The 67 zero-target effects use specialized/scripted handling and do not enter the missing-creature pig fallback. Their individual gameplay behavior is not certified by this audit.

All 59 client creature entries explicitly named as incarnations have valid display/model links. All 61 client shapeshift-form records have valid nonzero display/model links. These are structural checks, not proof of every incarnation's visual rendering or unlock behavior.

Eleven effects remain unresolved and unchanged:

| Spell | Reason |
| --- | --- |
| 8324 Perm. Illusion HE Hunter | Client target 6029 is named Lava Spout Totem; identity conflicts with spell name. |
| 35375 Dive Bomb | No recovered mapping for creature 20559. |
| 68662 Goblinated | No recovered mapping for creature 37925. |
| 73952 Motivate | Target is the invalid/sentinel value -1. |
| 77776 Lavamans Stalker | No recovered mapping for creature 41624. |
| 525072 Twisted Seal | Creature 190218 refers to display 840000 without a usable complete display/model chain. |
| 955175 Plank morph | Referenced Warden bridge model file not found in installed archives. |
| 986317 Death Visual Unused Aurastate | No recovered mapping for creature 10289. |
| 991034 Madness Transform #1 | Referenced Illidan Past model file not found in installed archives. |
| 993948 Falling! | Referenced Ogre boulder model file not found in installed archives. |
| 5011107 Transform | No recovered mapping for creature 49930. |

## Evidence and validation

- Upstream reviewed: 618a4181d5c0b5c7f5ddd9be67fae914d90561a4, jealous-sound/azerothcore-wotlk-coa. This is the community reconstruction, not Ascension's official backend.
- Exact upstream precedent: [Web Wrap cocoon repair, #6369](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/6369), commit 5069eed6dcf54905e9b427813f6e25d56e8913bf.
- Related upstream: [incarnation preview queries, #353](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/353) and [display streaming, #5794](https://github.com/jealous-sound/azerothcore-wotlk-coa/pull/5794). Neither substitutes for missing transform creature templates.
- Fallback cache reviewed: hertigservices/ascension-data at 8c2e2b22583785f22866261d0c7e540eabf032e4. Locally available records did not resolve the remaining exceptions; the separately downloadable full Exiles export was not imported for this batch.
- Client baseline: owner-designated Ascension A52 Free Pick installation, preserving Data/area-52/patch-D.MPQ. Installed files are not asserted to be the latest upstream release. Relevant extracted data hashes are recorded in provenance.json; raw client archives are excluded.
- tools/verify_all.py source checks passed. The focused area52_transform_models harness passed SQLite and disposable MySQL checks for all mappings, both Ascendance failures, repeated application, existing-data preservation and spawn exclusion. No C++ change or rebuild is required.
- In-game display, aura expiry and visual restoration remain tester acceptance checks. No live activation, source PR submission or launcher promotion occurred in this milestone.

The pending SQL is the complete server repair and is staged for the next authorized Area 52 activation. It must be included explicitly alongside other pending packages; older immutable server packages do not contain it.
