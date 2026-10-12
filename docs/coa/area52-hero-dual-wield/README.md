# Area 52 HERO Dual Wield

The starter kit includes two Worn Daggers (2092), but HERO baseline proficiency reconciliation omitted Dual Wield (674). The effective item has InventoryType 13 (one hand), and the equipment handler correctly requires Dual Wield for the offhand.

The integration patch extends the existing Area 52 baseline Block proficiency repair to grant both Block and Dual Wield permanently. The login caller runs after quest-spell cleanup and saves changes. It covers first login and existing characters, skips already learned or unavailable spells, uses the approved learning scope, and retains the existing Free Pick HERO restriction. It does not grant Titan's Grip or change equipment rules.

This patch targets the existing Area 52 integration source. Its prerequisite, Area52BlockProficiency.h and its login caller, is not in upstream main; it is not a standalone upstream PR. Publication base: 618a4181d5c0b5c7f5ddd9be67fae914d90561a4.

Validation uses the effective Area 52 source with tools/verify_all.py, build and freepick_advancement harness stages. The matching report accompanies this package. In-game acceptance remains pending. No live activation, client release, or launcher promotion is included.

Apply with `git apply --check docs/coa/area52-hero-dual-wield/runtime.patch`, followed by `git apply docs/coa/area52-hero-dual-wield/runtime.patch` in the matching integration checkout. Include the rebuilt binary in the next activation; the previously immutable Bulwark candidate predates this fix.
