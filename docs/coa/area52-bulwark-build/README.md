# Area 52 Bulwark build verification

The reviewed Bulwark build contains 69 Advancement entries and nine Mystic Enchants. This package repairs their missing connections and records structural verification for the full build: 53 VERIFIED entries and 25 existing CERTIFIED entries. It does not award new player certifications.

## Repairs

- Persistent Bulwark channel ownership, shield/full-energy admission, block energy, escalating drain, Shield Slam scaling, retaliation, proc suppression and cleanup.
- Wild Aggression, Shield Finesse, Redoubt chance, Anticipation block charges/cooldown, Shield Cover and First to the Fight cleanup.
- Shockwave child selection/AP scaling, Intervene's three-target taunt, Primal Precision's finishing-move cooldown helper, Mongoose Bite's ranged AP coefficient and Thunder Clap's two-handed bonus.
- Tank stance exclusivity/crit suppression/casting defense, Defensive Stance penalties, separate Shield Wall/Combat Insight PvE and PvP effects, and Avatar's fixed ground speed/root immunity.
- Tracking Mastery's six companion grants and ownership-gated Portal Mastery/Runeforging trainer lessons. Original trainer locations, prices and level requirements remain; no unrelated new class abilities are offered.
- Matching status tooltips and granted-ability certification/catalog eligibility, preserving existing SHIFT text and unrelated catalog entries.

## Evidence and scope

`audit.json` covers every build entry. `movement-review.json` compares client and effective server definitions for 306 rank/helper spells. Build and five focused harnesses pass. The isolated startup/export scenario passes natively and its affected bindings are reviewed; it is exploratory, so the runner correctly does not claim combined gameplay verification. SQL idempotency, source patch reproduction and complete client archive readback are recorded separately.

This is VERIFIED structural coverage: connections, grants, ownership persistence, rank upgrades and movement rules. Combat outcomes and reconnect behavior remain tester CERTIFICATION. No live restart, client installation, launcher asset release or Discord post is part of this source milestone.

Reviewed references:

- [CoA upstream main](https://github.com/jealous-sound/azerothcore-wotlk-coa/commit/b366fa8434d0a50a03c6d4cf0611eae37d99821e): native handlers, rank infrastructure and Wildcard/shared spell components. This is an independent reconstruction, not Ascension's official backend.
- Hero Free Pick PTR revision `826c9ee207313c0dafcc7b4b9ad37d655b4723d2`: mastery/trainer concepts, with Area 52 ownership rules retained.
- [Ascension data fallback](https://github.com/hertigservices/ascension-data/commit/8c2e2b22583785f22866261d0c7e540eabf032e4): recovered definitions cross-checked against effective server data and the primary Area 52 client.

## Integration package

The runtime repair is stored as a focused patch against the tested Area 52 integration source. `source-preconditions.json` records every required before/after source hash. This branch does not claim that applying it to clean upstream main produces the complete Area 52 realm, and is not an upstream PR.

Run `python apply.py <integration-source> --check`, then omit `--check` to apply. It refuses mismatched or partially updated source. Run the repository's `tools/verify_all.py` for build and the `area52_bulwark`, `area52_mystic_grant_ownership`, `freepick_advancement`, `area52_cast_movement` and `area52_hoplite_support` harnesses. The pending world SQL belongs with the matching server binary.

`patch_bulwark.py` and `bulwark-spells.json` reproduce the Spell.dbc edits from the matching existing client overlay. Archive hashes and changed records are in `client-changes.json`; no client archive is included here. Client packaging must preserve the published Spartan Kick repair. Merge current certification reports before activation so newer tester credit cannot be lost.
