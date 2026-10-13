# Feed Pet healing repair

Hopstrike knows Feed Pet 6991. The effective Area 52 client and server Spell.dbc define
its child 1539 as SPELL_EFFECT_HEAL_PCT with base value 49, meaning 50% maximum health.
The common feeding handler instead overrides effect zero with the food happiness benefit
(35000, 17000 or 8000), making the child use that value as a percentage.

Only periodic happiness energize effects now receive that override. Healing children use
their defined amounts. Food eligibility, consumption and combat restrictions are unchanged.
The happiness child 1101539 used by 1106991 retains its food-quality amount; 1356991 uses
the same healing child as 6991. Reviewed upstream: 4f66bddb12a32eeba49d42621651f19c46c4f570.

The native regression compiles the actual EffectFeedPet function with stub collaborators.
It fails before the correction and covers all three food tiers, defined healing, legacy
happiness, unsuitable food and dead pets. This is handler coverage, not an in-game combat
or reconnect test. A full worldserver build and unit tests run against a matching isolated
baseline with only this method transplanted; the initial mixed-revision build failed on
an unrelated GetSpellTwinFor API mismatch. In-game acceptance and activation remain pending.
