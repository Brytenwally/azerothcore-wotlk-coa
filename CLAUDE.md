@AGENTS.md

# Area 52 Mystic Enchant scripting

The owner sends a Free Pick build's Mystic Enchant panel. Check each enchant's status in the server's
`.mystic audit` export and make the unsupported ones work. "Connected" only means `LoadConnections` passed the
structural check (effects resolve to handlers); it does not prove the enchant behaves as its tooltip says, so read
the connected ones too and report any part that does nothing.

## Workflow

- **Read Spell.dbc for every spell an enchant scripts or grants, however long it takes** (owner's rule). Resolve
  every tooltip line in the data: the enchant's own effects, every spell it triggers or names (`$<id>s1`,
  `@s:<id>@`), hidden helpers and every rank or copy. Search by name across the whole file, not only the ids the
  enchant names: helpers are often named `<Spell> SLS`, `<Spell> iCD`/`ICD` or `<Spell> PPM`, and the neighbouring
  ids usually hold them. Ask the owner only what the data leaves open, and report findings before coding.
- Data sources: the server's `Spell.dbc`, read with `apps/coa-dbc` (`inspect_dbc.DataSet`); `MysticEnchant.dbc`
  (field 1 spell, 3 quality, 5 level, 7 Worldforged, 8 non-zero, 13 flag `0x200` usable); and the owner's client
  DBCs: `SkillLineAbility.dbc` (field 1 skill line, 2 spell) for spellbook tabs, `SpellRank.dbc` (field 1 first
  spell, 2 spell, 3 rank) for rank chains.
- One build = one commit named `<legendaryenchantname>-build` on `main` (for example `bloodandguts-build`); later
  fixes are `<legendaryenchantname>-build: <topic>`. Put each change's SQL in a new
  `data/sql/updates/pending_db_world/` file rather than editing one already pushed (the owner may have applied it),
  and hand the owner a copy to apply at once.
- Code layout per build: `src/server/coa/Area52<Build>Rules.h` (ids and pure functions) and
  `src/server/coa/Area52<Build>.inc`, included by `AscensionMysticEnchant.cpp` (after `GridNotifiersImpl.h` when it
  searches the grid). Add `HasArea52<Build>Handlers(spell)` to the handler list in `LoadConnections` (checked before
  the grant path) and register the scripts in `AddSC_AscensionMysticEnchant`. The harness
  `apps/coa-tests/area52_<build>/run.py` compiles the rules header and checks the grant and SQL rows.
- Verify with `python -B tools/verify_all.py --stages source,harness --harness area52_<build> --base origin/main`,
  plus `codestyle-cpp.py` and `codestyle-sql.py --files`. Nothing is tested in game by Claude; say so.

## Spell grants (`AscensionMysticSpellGrants.h`)

- `{Enchant, Spell, Requires, Replaces}`. `Requires` is a spell whose rank chain must be known;
  `Replaces` puts the granted spell on the replaced spell's button (`SetTemporarySpellReplacement`). Only replace
  when the owner says so (Dragon Warrior: Dragon Roar for Death Wish, Dragon Charge for Intercept); the other
  grants sit beside their requirement.
- The enchant's DUMMY effect names the grant: MiscValue = required spell, MiscValueB = granted spell. Grant that
  id: it is the one with a `SkillLineAbility.dbc` row in its class's skill line, so it lands in that class's
  spellbook tab. The copy the tooltip shows (`@s:`) often has no row and lands in General (Compound Shot 826679,
  not 984925; Disembowel 947470, not 990240).
- Granted spells with ranks need a `spell_ranks` chain (copy it from `SpellRank.dbc`; talent chains come from
  `Talent.dbc` already). Grants learn every rank whose `SpellLevel` the character has reached (`GrantRankDue`).
- Hidden helper auras that Ascension applied server-side (a passive `... SLS` with a proc) can be granted as
  passive spells (Executing Disembowel 977908). `RetiredSpellGrants` removes a grant that was replaced.
- `Replaces` follows every rank of the replaced spell and points it at the highest granted rank, so it works when
  the character knows a higher rank of either. An enchant that needs a talent (`@unlockby:` on the enchant) is listed
  in `EnchantUnlocks`; its grants stay off without that talent (Knight of the Eclipse needs Eclipse 48525).
- A swap shows the replacement in the original's spellbook entry, so `SetTemporarySpellReplacement` hides the
  replacement's own entry while the swap lasts and restores it when the swap ends. `Synchronize` undoes a grant's
  swap before unlearning the granted spell and sets it after learning, so neither entry goes missing or doubles.
- `SetTemporarySpellReplacement` needs both spells known, so a temporary swap must grant the replacement too
  (Combo Shot 850515 for Compound Shot's recast window 826680).

## Procs on this core

- `SPELL_ATTR3_SUPPRESS_CASTER_PROCS` on a cast stops every hit proc of that cast. Slam 1464, Execute 5308,
  Mutilate 1329 and Judgement of Light 20271 carry it, so they never proc on hit.
- Their damage spells are triggered with `SPELL_ATTR3_NOT_A_PROC`, which on this core means a triggered spell that
  *may* proc. Point `spell_proc` at the child's family bits: Slam 50782 (family 4, mask0 `0x200000`), Execute 20647
  (family 4, mask0 `0x20000000`), Mutilate hits (family 8, mask1 `0x2`/`0x4`), Judgement damage (family 10, mask0
  `0x800000`). A triggered spell without that attribute procs only auras with `SPELL_ATTR3_CAN_PROC_FROM_PROCS`.
- Cast-phase procs fire for any non-triggered cast but have no action target.
- Spell-family masks can miss the spell they mean: Syndicate Sniper Training's damage part and Hone In point at
  bits Syndicate Sniper Shot lacks, and the shot shares all of Ambush's bits. Add such a bonus in the spell's own
  script instead of widening the mask.

## Recurring Ascension patterns

- Internal cooldowns are hidden auras: Sudden Death's is "Sudden Death iCD" 411167 (6 s); its Rare shortens that
  aura with a DURATION spellmod. A proc that "ignores the cooldown" casts the effect without checking the aura.
- Worldforged Rare powers (Necrotic, Draconic, Primal ...): the hidden `<Enchant> SLS` holds the chance
  (`ProcChance`), the power buff applies the shared 20 s cooldown 780999 itself, and the value is the enchant's
  EFFECT_0 amount (it stacks with the enchant). Scripts: `aura_area52_draconic_sinister`,
  `spell_area52_necrotic_execute`.
- Ascension spell-mod ops 36 (vs players) and 38 (vs NPCs) are beyond the core's 32 ops and silently ignored. List
  the spell in `ApplyArea52MysticTargetDamageContracts` (`AscensionConditionalCombat.cpp`), which turns them into
  damage-versus-target auras; check every "+X% damage" buff for them (Deadly Combo, Solar/Lunar Knight).
- Ascension cooldown effect 165 names the original spell; `EffectAscensionModifyCooldown` sends it to a temporary
  replacement (Skull Crusher cools down Dragon Roar).
- Spell.dbc keeps stale aura types on effects that apply no aura (Piercing Cleaver's TRIGGER_SPELL effect carries
  aura 42). The connection check reads aura fields only on aura effects; when an enchant shows unsupported, look for
  such leftovers before assuming a missing handler.
- Engine support: aura 262 (ignore aura state) and 286 (periodic crit) work; aura 337 (spell override) and effect
  178 have no handler, so script them (Dragon Charge, Compound Shot recast).
- Coefficients come from tooltip formulas. `spell_bonus_data` `ap_bonus`/`direct_bonus` also apply to melee-class
  spells; for a physical spell "SP" is physical-school spell power, usually near zero. Terms the native data cannot
  express (Disembowel's AP x combo points squared) go in `DoEffectCalcAmount`. Combo points are still on the caster
  in `ALLSPELLHOOK_ON_BEFORE_EFFECTS` and during the hit; they are cleared in the finish phase, which for an instant
  spell runs before `ALLSPELLHOOK_ON_CAST`, so never read them there (Deadly Combo always saw 0).
- "Next X" buffs without charges must be removed by a script on X (Execute spends Deadly Combo, Judge, Jury,
  Executioner and Weakness).

## Builds done (2026-10-28)

`compoundpower-build` (Marksman), `dragonwarrior-build` (Dragon Warrior), `bloodandguts-build` (Gutripper) and
`knightoftheeclipse-build` (Knight of the Eclipse), with their follow-ups. Harnesses: `area52_marksman`,
`area52_dragon_warrior`, `area52_gutripper`, `area52_knight_of_the_eclipse`.
