# XP hotspot in-game acceptance

Run this on the test realm after the combined client/server update. Record the tested source commit,
server binary hash, database migration name/hash and client archive hash. Keep production activation
and upstream PR publication separate from these tests.

October 11 boundary/editor refinement: offline source checks, native build/unit stage, C++ geometry
harness (including every imported polygon), 13 mocked Lua UI tests, data-import tests, and disposable
MySQL migration/reapply tests passed. The four new UI regressions fail against the preceding release
and pass against this candidate. C++ and SQL style checks passed. These results do not establish
rendered-client, live GM concurrency, or multiplayer acceptance. The table below remains the in-game
gate; the upstream PR is held until those checks and full Main qualification are complete.

| Check | Expected result |
| --- | --- |
| Feature disabled | No hotspot controls, bonus or rotations; ordinary map behavior is unchanged. |
| Azeroth-only configuration | No Outland/Northrend event can start, including one queued by a GM. |
| Ordinary account sends GM requests | Server rejects them; no location or announcement setting changes. |
| Two GM characters edit the same record | First save succeeds; stale second save is rejected. |
| Two characters on one GM account | Drafts remain separate; one logout does not discard the other's draft. |
| Create circle on a viewed map while character is elsewhere | Saved world position matches map placement; ordinary clicks still zoom normally. |
| Polygon with CTRL clicks, undo, then save | Simple boundary saves; crossing or degenerate boundary is rejected. |
| ALT-select polygon vertex, CTRL-move, then save | Only the selected vertex moves; invalid crossings are rejected. |
| SHIFT-select nearby areas and merge | Preview preserves all source shapes and gaps; Save retires sources atomically; Cancel changes nothing. |
| Another GM edits a selected merge source before Save | Entire stale merge is rejected; no sources are retired. |
| Merge whole zones or areas from different maps/zones | Server rejects the request without changing existing records. |
| Revised catalog | Area-based names, no retired holiday/duplicate entries, 106 default Barrens entries before GM edits. |
| Type rapidly in search and clear it | One debounced current-term query; page resets to 1; no old result becomes the final result. |
| Bear Cave map panels | Both panels have dark-blue backgrounds, gold borders, readable fields and continuous search border. |
| Whole-zone hover | Highlight follows the sampled terrain boundary; XP uses actual zone membership. |
| Icon/highlight disabled | Visibility follows settings; GM catalog still permits selection/editing. |
| Search disabled and other-map locations | Name/ID search, pagination and selection work without losing records. |
| Enter/leave circle or polygon | Buff appears/removes promptly; an immediate outside kill receives no stale bonus. |
| Same creature reward inside/outside | Eligible kill XP increases by 150% once, accounting for other multipliers. |
| Overlapping 150% and 200% areas | Highest bonus is 200%; it does not stack to 350%. |
| Quest/profession XP | Hotspot does not increase these rewards. |
| Party members inside/outside | Each receives only the bonus applicable at their own location; normal group sharing remains. |
| No-bonus-XP challenge | No positive hotspot XP is applied. |
| Death, resurrection, teleport and instance entry | Buff remains presence-bound; no bonus leaks into instances/BGs. |
| Relog inside/outside and server restart | No persisted personal buff outside coverage; active event retains its original end time. |
| Expired event after downtime | It is retired; a fresh full duration is not granted to the old event. |
| Queue, stop, disable, cooldown, duration edit | Each obeys documented eligibility and updates clients once state commits. |
| Announcement toggle | One message per new start when enabled; none on restore or while disabled. |
| Multiple clients with map open | Changes converge; no packet/chat spam, errors, frozen map or stale editor state. |
| Combined client update | Existing client features and other included fixes remain present after packaging. |

Use short one-minute durations and nearby custom areas for the first rotation/boundary tests. Keep
the one-hour defaults for the seeded catalog. Reset test-only location settings after acceptance.

Upstream PR gate: complete the multiplayer/lifecycle checks, preserve evidence and limitations,
run full upstream qualification, review the focused diff, then publish the standalone PR. Do not
include unrelated Area 52 client fixes, progression changes, launcher assets or private test data.
