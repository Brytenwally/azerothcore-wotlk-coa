# Area 52 granted-ability certification

Source-granted spell families can be certified independently of their granting Mystic Enchant or
Advancement selection. IDs use the reserved `0x40000000 | rootSpell` namespace, with no database schema
change. The server checks VERIFIED eligibility and requires a currently learned family rank before
accepting certification. Existing account credit, persistence, duplicate prevention and broadcasts
remain in use. Native Advancement entries retain their existing identity.

This is an extension to the existing Area 52 certification deployment, not a standalone upstream-main
implementation. `granted-abilities.patch` applies to the deployed source versions identified in
`source-preconditions.json`. Use `git apply --check` before applying. Install `run.py` at
`apps/coa-tests/area52_granted_abilities/run.py` in that integration checkout.

The matching client is in CWO4PapaBear/Area52-FreePick-Client, documented in
`docs/granted-abilities.md`. Its generator emits the catalog and certification ledger together.
Supply the effective audit and current status evidence, retain existing certifications and deploy the
generated ledger at the configured CoA.FreePick.CertificationFile path. Activate the rebuilt server,
ledger and client package together. Source publication does not deploy or release these components.

The client catalog currently covers 48 recovered grant families. Fifteen additional families qualify
for independent VERIFIED records using existing per-rank audit evidence; source certification is never
copied to a child. Unearned abilities are searchable under the appropriate filters but cannot be
learned or certified through their source-managed rows.

Checks: Area 52 incremental build and `area52_certification` plus `area52_granted_abilities` harnesses
passed through `tools/verify_all.py --stages build,harness`; C++ style passed. The harness checks
namespace isolation, family ranks, eligibility and invalid ledger rejection. Client Lua 5.1 tests,
archive readback and syntax checks passed. Actual client visual acceptance remains pending.
No simulated combat or reconnect test was required or claimed.

Upstream reviewed base: 231b9958f (achievement safeguard merge), following a58f8bf60.
No upstream client package was imported. Existing Area 52 client/server dependencies are required.
Package is staged only; no restart, live installation or tester release was performed.
