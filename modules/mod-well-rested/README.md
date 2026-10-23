# Well Rested

Spend 15 continuous online minutes alive and out of combat inside an inn to earn
8% additional monster-kill experience for two online hours. Leaving the inn,
combat, death or logout resets unfinished rest. Earned time pauses offline and
survives death. Each completed inn cycle starts another cycle and refreshes the
reward without stacking it. Capital-city rest alone does not qualify.

The module uses per-character state and existing server hooks. It requires no
other module. Quest, exploration and player-kill experience are unaffected.
The two native countdown auras are display-only; the experience hook applies
the bonus once.

## Server integration

The normal module loader discovers `src/WellRested.cpp`. The normal character
database updater applies `rev_20260927_00_well_rested.sql`, creating two
module-owned tables without changing core tables. The configuration template
is installed by the module configuration machinery. Missing schema or invalid
configured spells prevents the module from enabling.

Defaults are disabled and chat-only so merging does not activate gameplay or
reference spells absent from an existing client. The accepted deployment uses:

```ini
WellRested.Enable = 1
WellRested.RestSeconds = 900
WellRested.RewardSeconds = 7200
WellRested.BonusPercent = 8
WellRested.Announce = 1
WellRested.RestingSpell = 910100
WellRested.RewardSpell = 910101
```

Configuration changes require a restart. Disable with `WellRested.Enable = 0`;
retain the character tables to preserve earned time.

## Matching client data

Both server and clients require matching display spell definitions. The
included generator preserves existing rows and refuses conflicting spell IDs:

```sh
python modules/mod-well-rested/tools/build_display_dbc.py current/Spell.dbc staged/Spell.dbc --rest-icon 72961 --reward-icon 7415
```

This reproduces the accepted deployment's spell definitions. Its icon mappings
are `72961` to `Interface\icons\INV_Ascend_Tavern_67` and `7415` to
`Interface\Icons\achievement_zone_jadeforest`. Those textures and mappings must
exist in the target client. Duration rows `347` and `367` must provide 900000
and 7200000 milliseconds. Check effective client/server DBCs, world `spell_dbc`
and spell bindings for collisions before allocating `910100` and `910101`.

Omitting the icon arguments uses stock icons `44` and `117`, with identical
mechanics but different artwork. The repository contains no proprietary client
archives or textures. Client packaging/distribution is not performed by a
server build. Integration with CoA's client release process remains required
before this contribution reproduces the accepted presentation automatically.

## Persistence and verification

Earned remaining milliseconds and fractional experience are saved on reward
transitions, character saves, logout, and every minute while active. Inn
progress is not persisted. Abrupt termination can restore time since the last
successful save, normally up to a minute. Synchronous writes preserve logout
ordering but still require load testing with the target database.

Timer regression tests register with the normal unit target. Display-data and
isolated schema tests accompany the module. Run repository verification through
`python -B tools/verify_all.py --base origin/main`; the isolated schema test
requires a disposable MySQL Docker image and is unavailable without that input.

The owner reports the deployed implementation working as intended. Previous
stock and CoA module builds and isolated schema tests passed. These do not
replace compilation and multiplayer lifecycle testing against current CoA
main. Current-main runtime, concurrent database load, cross-module experience
ordering, and client distribution remain merge-readiness gates.

Source: [mod-well-rested](https://github.com/CWO4PapaBear/mod-well-rested),
revision `6e35803419f0681cc0d28ac3b40467b085f0df49`. GPL-2.0-or-later;
see `LICENSE`.
