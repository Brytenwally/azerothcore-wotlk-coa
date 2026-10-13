# Remorseless Attacks

Reviewed against upstream main 4f66bddb12a32eeba49d42621651f19c46c4f570 and the effective Area 52 client/server definitions.

Advancement entry 502 comprises talent ranks 14144/14148 and triggered buffs 14143/14149.
The native kill proc requires an XP/honor-eligible victim before its early kill-event acceptance.
Consequently the differing generated family masks do not block rank 2 and are not changed.
The buffs supply 20/40 percentage points of critical chance for 20 seconds, with the existing
single-charge proc and masks for the listed attacks (including Mutilate's child strikes).

The missing rank-2 capstone now runs after the existing trigger effect. It refreshes only an
existing self-owned Slice and Dice to its maximum combo-point duration, retaining duration
modifiers. This uses the established upstream Cut to the Chase implementation. It does not
create Slice and Dice, replace the normal critical-chance trigger, or change rank 1.

Both talent ranks are passive; both triggered buffs are instant and non-channelled. No new
player-cast spell is granted, so no movement interruption bits are appropriate. Advancement
retains its existing selection persistence and talent-rank path; trainer books do not grant
these talent ranks. Live spell_proc and spell_script_names were inspected before the repair.

The focused regression compiles the actual handler body with mocked aura collaborators and
checks absent buffs, rank isolation, duration modifiers and repeated refresh. The server build
checks real API and script registration compatibility. Neither is in-game certification.

Apply the SQL script binding with the matching binary. Staged yellow VERIFIED tooltip changes
cover all four spell IDs; merge advancement entry 502's status into the current certification
ledger at activation, preserving any newer CERTIFIED report. Merge the tooltip edits into the
latest pending client payload rather than replacing it with an older full archive.

No live activation, tester release or restart is part of this source change.
