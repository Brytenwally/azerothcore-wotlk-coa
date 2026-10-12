# Advanced training dummy test implementation

This is an independent reconstruction for Area 52 testing, not recovered Ascension server code.
No city spawns or existing training dummy bindings are replaced by this package.

## Sources

- Ascension August 20, 2023 changelog: https://ascension.gg/de/changelog/1?page=71
- Configurable Model #001 quotes: https://db.ascension.gg/?npc=967171
- Owner handoff and historical execute documentation: https://ascension.gg/en/changelog/1?page=170
- Local creature-cache research: `hertigservices/ascension-data` snapshot
  b615898e8f48b9d00ce1d35d94f7bacab28df8c9617563dc9c943d3e6106c10c.
- Independent upstream reviewed at 0facf53ae0a68369bfaac845a51a0b369c71777b.

## Implemented test behavior

- Nonlethal incoming damage, stationary combat, and per-attacker idle cleanup. Accepted damage is
  retained for combat reporting, capped below lethal damage rather than discarded.
- Execute targets stay at 18% health after being attacked, then restore on disengage.
- All five roles acquire the first player's level and creature-base stats; pet attacks resolve to their owner.
  A dynamic target is reserved to that player until release and resets to level 1 afterward.
- Standard targets in an explicit creature formation share ownership, level acquisition, and activity timeout.
  Cleave and AoE stations reuse the standard template; proximity alone does not join unrelated stations.
- Model #001 supports information, idle vulnerability toggle, owner-only level adjustment and crowd-control
  immunity toggle. Combat end restores default crowd-control immunity.
- Healing targets accept effective healing after `/poke`, enter combat for meters, and refresh their health
  deficit. A second `/poke` stops practice and clears auras/combat. Disabled healing is zeroed.
- Tank targets use ordinary melee timing and damage, allowing the normal mitigation, rage and attack-speed
  aura paths. They do not chase. `/poke` by the owner stops practice; range/death/logout releases ownership.
  A lethal hit is capped to leave one health and stops attacks.

## Explicit limitations of this test version

- Tank simulated-death/overkill and on-death procs are **not implemented**. Calling the normal death path
  would kill players and can trigger resurrection, durability, achievement and encounter side effects.
  The safety cap must not be presented as equivalent to the historical simulated-death behavior.
- The five original templates match the player level. The additional Boss prototype described below
  acquires player level +3. Expansion stat tiers follow the acquired level; no fixed expansion copies are installed.
- Stat magnitudes use the reconstruction's creature base-stat tables, not a recovered original damage
  formula. Tank intensity, armor, healing behavior and simultaneous-user behavior require owner acceptance.
- Healing dummies remain targetable for `/poke`; disabled healing is rejected. This is not a full client
  reproduction of the historical inactive presentation.
- A five-second idle timeout, 60-yard disengage radius, 50% healing deficit, and level-adjustment bounds
  1–83 are explicit reconstruction defaults rather than recovered constants.
- No simulated combat tests or in-game acceptance are claimed. Compile and source checks do not prove
  defensive threshold proc timing, effective healing meter behavior, or scaling on the first attack.

## Owner test checklist before city placement

Use temporary GM spawns in an isolated area. Test 967254 (single/cleave/AoE), 666925 (execute),
666935 (healing), 967182 (tank), and 967171 (configurable).
Repeat dynamic tests at different levels and with pets. Check independent users, loss of combat after
stopping, aura cleanup, level reset, tank rage/slows/nonlethal stop, and healing activation/meters.
No permanent placement SQL is included. Do not enable automated city replacement until behavior and
the separate surveyed placement plan have been accepted.

## Boss prototype (9780200)

Staged October 11, 2026; not activated or placed. The existing tank and other roles remain available.
The owner requested either a stun or a disarm as the heavy-strike counter.

- Uses the tank session, stationary attacks, owner-only `/poke` stop, and nonlethal outgoing damage.
- Acquires the engaging player's level +3, using creature class/level health, armor, attack power and
  base damage. The supported player range is 1–80 (dummy levels 4–83).
- At six seconds, begins NPC Shadow Bolt (9613), a three-second cast with its existing client cast bar. Subsequent
  attempts are 18 seconds apart. The prototype supplies twice its maximum melee damage as the spell's
  base damage. It bypasses resource cost only; casting, interrupts and school lockout remain native.
- At twelve seconds, sends the owner a boss whisper: heavy strike in three seconds, stun or disarm.
  This repeats every 18 seconds. Auto-attacks pause during the wind-up. The warning waits for an
  existing cast or stun to end; the spell similarly waits for the heavy-strike window to finish.
- Stun and disarm immunity open only during the warning. Either aura cancels that pending strike;
  a normal spell interrupt does not cancel it. Other configured crowd-control immunities stay enabled.
- If not countered, executes one melee swing at triple its normal damage modifier. Native melee hit,
  dodge/parry/block, armor and absorb processing remain in use. Leaving melee range avoids the hit.
  The modifier and attack timer are restored immediately, so the next ordinary swing is not empowered.
- Spell-school lockout delays the next spell attempt; reset cancels pending actions. Leaving range,
  logout, death and reaching the nonlethal safety limit stop the Boss session.

### Scaling provenance and limitations

Reviewed independent CoA upstream `main` at `4b25cd5d56d353061075916ee6c6f071b57d0465`.
`Creature::getLevelForTarget` supports a configurable world-boss level difference. That supplies
combat level, not a universal boss health or damage curve. `CoACreatureScalingPolicy.h` provides
context multipliers; Destiny Weaver separately provides per-viewer level/stat scaling and excludes
world bosses, raids and non-normal dungeon difficulties. Dungeon-specific authored health and
Mythic modifiers are different systems. These are independent reconstruction code, not Ascension's
original server implementation.

The effective local Area 52 patch-D SHA256 was
`86598748c5f2ed16ac4034afc29d82c64edd6e3407fc743619dc9ba3d238d1d4`.
Its client definitions include Tank Dummy Auto Attack (2180272), triggering 2180273, and Fierce Blow
(975011). The captured MobSpells data records these attacks on historical tank dummies, but does not
establish a complete original level-to-boss-damage formula. The current test prototype therefore uses
explicit alpha tuning, not claimed original Ascension parity. No client asset changes are required.

Build and the `boss_dummy` focused extracted-handler harness passed. The harness checks rotation,
stun/disarm cancellation, lockout deferral, range, reset and restoring the melee damage modifier.
Visual warning/cast-bar presentation, first-hit scaling, actual player mitigation and nonlethal behavior
still require in-game acceptance. This prototype does not introduce death/overkill proc simulation.

### O'mrogg hologram appearance

The staged Boss Simulator uses Warbringer O'mrogg's display 18031 (model 2401), preserving the display's
native scale, and his normal weapon item 29484. Only simulator entry 9780200 changes; the dungeon boss
and the other training dummies remain unchanged.

Cosmetic aura 2180104, `Cosmetic - Ghost Visual Blue Fog Transparent`, exists in both the effective
client and server Spell.dbc. It has one apply-aura dummy effect, visual 886, indefinite duration 21,
and no stat, invisibility, phase or transform effect. No spell_script_names override was present.
The simulator applies it on reset and restores it on maintenance after aura cleanup without stacking
copies. The focused harness checks restoration and exclusion of other dummy roles.

The expected blue translucent appearance still needs visual acceptance on O'mrogg; it is based on
the client definition, not a rendered preview. The existing client already has these definitions.
No mechanical projector-pad placement is included in this visual step.

### Permanent movement protection

Standard and advanced training-dummy constructors now apply effect immunities for knockback and
pull variants, mechanic immunities for fear, horror, disorientation and charm, and aura immunities
for fleeing, confusion, charm and possession. Disabling combat movement alone did not prevent these
effects. The movement protection uses a separate immunity source from configurable crowd control,
so `/clap` and the Boss Simulator's stun/disarm window cannot remove it. Damage-school immunity is
not added; the damaging portion of abilities such as Thunderstorm remains eligible to hit.

Reviewed upstream main: `b366fa8434d0a50a03c6d4cf0611eae37d99821e`. The Thunderstorm handler uses
`SPELL_EFFECT_KNOCK_BACK`; the core stores immunity by effect/type and source. The new constructor
regression fails against the previous source. Candidate **VERIFY ALL: PASSED** for the server build
and `boss_dummy` harness; separate source and C++ style checks passed. Reports are under
`outputs/Area52_Dummy_Movement/{candidate,source}/report.json` in the working environment. The harness
checks retained movement immunity across configurable CC toggles and boss counter windows, alongside
existing rotation and reset checks. Live spell casts were not run; runtime flakiness is unmeasured.
No SQL or client update is required. This source repair is staged for the next server activation.


### Goblin simulator base

Entry `990001`, Boss Simulator - Goblin Teleporter Base (3x), is a separate decorative
clone of `188454` at three times its template scale. The original stays unchanged.
After loading the template at server startup, stand at the intended center and use
`.gobject add 990001`. No spawn is included; placement remains a GM choice.
