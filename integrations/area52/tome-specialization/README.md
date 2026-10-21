# Area 52 Tome specialization repair

This independent feature branch carries an exact-baseline source patch for the existing Area 52 realm. It does not import the Area 52 backend into upstream and is not part of the Prestige draft PR. No activation is performed by this branch.

Apply `repair.patch` only to the Area 52 source matching `baseline.json` (LF-normalized SHA256). It publishes enchant presets for all contiguous unlocked build specialization spells, preserves existing native-slot entitlements and stored enchant contents, and handles independent selection of unlocked presets. Existing unlocks are recognized on login; no additional Tome is consumed. Current safe-state guards remain enforced. Until an explicit enchant selection is stored, the prior native active-slot behavior remains the fallback. Other realms are excluded by the existing Area 52 gate.

This repair does not implement separate enchant-token purchases or optional build-to-preset linking. It adds no client files, addons, SQL migrations or collection changes.

Verification: mount this directory at `apps/coa-tests/tome_specialization` in the matching Area 52 checkout, apply the patch, then use `python -B tools/verify_all.py --stages harness --harness tome_specialization`. Build and unit stages must run against that same Area 52 candidate. Do not run the harness directly or apply this patch to upstream's different backend.

The native extraction regression covers twenty unlock levels, preset wire payloads, selection and saved-setting reload, legacy contents, locked/malformed/unsafe requests and other-mode isolation. Engine/persistence/aura operations use adapters; this is offline engineering evidence, not gameplay certification. Source build and unit checks passed on the exact candidate. Prestige's separate integration patch must use this repair's after-hash as its before-hash, then validate and reset the active enchant preset rather than the native specialization index.
