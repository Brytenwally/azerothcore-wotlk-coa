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
- An enchant connected through a build's `HasArea52<Build>Handlers` keeps its DBC `StackAmount` as the copy limit,
  as native connections do; the older equipment, Hoplite and Battlemage handlers still allow one copy.
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
  `Talent.dbc` already). Grants listed in `GrantRanksFollowLevel` (Compound Shot and the builds since Gutripper)
  learn every rank whose `SpellLevel` the character has reached (`GrantRankDue`); add each new ranked grant's
  enchant there. The other grants learn only the listed rank.
- Hidden helper auras that Ascension applied server-side (a passive `... SLS` with a proc) can be granted as
  passive spells (Executing Disembowel 977908). `RetiredSpellGrants` removes a grant that was replaced.
- `Replaces` follows every rank of the replaced spell and points it at the highest granted rank, so it works when
  the character knows a higher rank of either. An enchant that needs a talent (`@unlockby:` on the enchant) is listed
  in `EnchantUnlocks`; its grants stay off without that talent (Knight of the Eclipse needs Eclipse 48525).
- A swap shows the replacement in the original's spellbook entry, so `SetTemporarySpellReplacement` hides the
  replacement's own entry while the swap lasts and restores it when the swap ends. `Synchronize` undoes a grant's
  swap before unlearning the granted spell and sets it after learning, so neither entry goes missing or doubles.
- A Free Pick talent whose spell only teaches others (Tree of Life 65139 teaches the form 33891 and 5420, restored
  in `AscensionStockScriptShapes.h`) cannot be learned itself: `Player::_addSpell` refuses spells with a learn
  effect. `LearnEntrySpell` learns the taught spells instead, so grants that require the form (Tree of Wrath) work.
- Shapeshift forms are the exception: the shapeshift bar lists learned forms, not superseded entries, so a form swap
  (Tree of Wrath for Tree of Life) hides the original form and leaves the replacement's own entry.
- `SetTemporarySpellReplacement` needs both spells known, so a temporary swap must grant the replacement too
  (Combo Shot 850515 for Compound Shot's recast window 826680).
- **Modifiers follow the replacement** (owner's rule): unless the owner says otherwise, every enchant or talent that
  modifies a spell also modifies the spell that replaces it (Transcendental Embrace buffs Chain Heal, so it buffs
  Living Lightning too). Replacements often share the original's family flags, so masks carry over by themselves;
  check them, and extend scripts keyed to the original (`spell_sha_chain_heal` on Living Lightning) or mask-less
  bonuses (the extra Chain Heal jump) to the replacement.

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
- Ascension forms beyond the core's `ShapeshiftForm` enum (Tree of Wrath is form 48) need an enum value and the
  matching cases in `HandleAuraModShapeshift` and the form switches; the owner's client `SpellShapeshiftForm.dbc`
  gives their flags (form 48 copies Tree of Life's). `AscensionTreeOfLife.cpp` holds the tree forms' cast rule.
- Cooldown spellmods (op 11) apply only to spells that already have a cooldown, so a "+X s cooldown" on a spell
  without one (Lightning Bolt in Tree of Wrath) needs `AddSpellCooldown` in a cast script. Hidden helpers often carry
  dead values: an amount of 0 (Tree of Wrath - Hit Chance) or an empty mask; take the tooltip's number instead.
- Effect 165 names one spell id, usually rank 1, while the cooldown sits on the rank cast; the effect now walks
  the named spell's rank chain (and each rank's replacement), so Ascension's cooldown helpers reach every rank.
- A crit chance set in `OnSpellCritChance` is still reduced by the target's crit-taken modifiers (resilience); a
  hit that must crit sets `TargetInfo::crit` in `OnSpellCalculatedTarget` (Unstoppable Force, Dark Intent).
- When a replacement does not share the original's family flags (Dragon Roar for Death Wish), widen the modifiers
  aimed at the original with `DoEffectCalcSpellMod` (Intensify Rage, enchant and talent).
- Many hidden helpers have ProcFlags 0 and no `spell_proc` row, so they never fire: give them a row or a script
  (Tree of Wrath's Lightning Rod aura 86436, Nature's Flow, Charged Tides).
- `SPELLVALUE_FORCED_CRIT_RESULT` only shows a crit; for a real one set `TargetInfo::crit` in
  `OnSpellCalculatedTarget`. "Crits when the trigger crits" pairs that with the helper's `CANT_CRIT` copy for the
  non-critical case (Dark Intent 980050 / 990248).
- A spell that spends a buff's spellmods lists the buff in `Spell::m_appliedMods` until the cast phase procs, so
  `ALLSPELLHOOK_ON_BEFORE_EFFECTS` can tell a Shadow Bolt cast through Nightfall (Shadow Trance 17941) or a heal
  under Nature's Swiftness.
- Ascension effect 169 (spread aura) has no handler (78 spells use it); Pestilence copies Corruption in its own
  script, as the owner chose.
- A buff whose DBC `ProcCharges` the owner does not want (Evoker's stacks) gets `Aura::SetCharges(0)` after each
  cast; a `spell_proc` row cannot clear it, because `Charges` 0 there falls back to the DBC value.
- To give one cast a bonus that lands at its hit (a missile's crit damage, a Fireball that spent Brain Freeze),
  remember the `Spell*` in `ALLSPELLHOOK_ON_BEFORE_EFFECTS` and compare it with `Player::m_spellModTakingSpell`,
  which is the hitting spell during delayed hits too. Clear the claim when a new cast reuses the pointer.
- Crit damage bonuses that depend on the target (Shatter: 25%, 10% versus players) go through
  `GetArea52CritDamageBonusPct`, called from `Unit::SpellCriticalDamageBonus` on the bonus part.

## Talent audit (requested 30/10)

The owner names a talent tree; audit every rank of every talent in it against the DBC and fix the scripting where
it contradicts the data. Order: classes alphabetically, Death Knight last; within a class, trees alphabetically.
One commit per tree, `talents-<class>-<tree>` (`talents-druid-balance`), with a harness `area52_<class>_<tree>`.

- The tree's talents and their rank spells come from `CharacterAdvancement.dbc` (type `Talent`/`TalentAbility`,
  field 32 class type, 33 tab). Rank chains come from `Talent.dbc` (`SpellMgr::LoadSpellTalentRanks`), not
  `spell_ranks`; check that the Free Pick ranks equal the `Talent.dbc` chain, since negative-id bindings and
  `GetAuraEffectOfRankedSpell` follow it.
- Compare with the owner's live database rows (`spell_proc`, `spell_script_names`, `spell_bonus_data`,
  `spell_linked_spell`, `spell_group`, `spell_custom_attr`), not the repository SQL: the live database ran migrations
  the repository no longer has.
- Report per talent: matches the DBC, conflict (fix), needs the owner's decision, or capstone nothing implements.
  The owner decides whether capstones get new scripting.
- Old WotLK hard-coded talent rules in `Unit.cpp`/`SpellEffects.cpp` find talents by `SpellIconID`; Ascension's
  copies often carry new icons (Celestial Bane 24674, not 1771), so those rules silently skip them. They can also
  double an effect the Ascension data now holds as a spell modifier (Brambles' Thorns bonus).
- A spell modifier without a class mask reaches every spell of its family unless `SpellAffect.dbc` names its
  targets; on talents that inflates every other talent's values (the Soul Leech overflow). Route such modifiers in
  `Area52DruidTalents::ModifierTarget` (or the class's equivalent) to the spell the tooltip means: Moonglow's
  +25/50/75 is its buff's regeneration while casting (965818 effect 2).
- Capstones are implemented when the owner asks (Balance, 30/10): `Area52DruidBalanceCapstones.cpp` holds an
  `ALLSPELLHOOK_ON_HIT_RESULT` script for hit-driven bonuses (extra Moonfire tick through `AuraEffect::PeriodicTick`,
  Roots silence, Wrath of Cenarius), aura scripts for dispel, apply/remove and expire bonuses, and `spell_proc` rows
  for capstone procs whose DBC has no proc flags. Internal cooldowns use `AddSpellCooldown` on the capstone's id.
- Free Pick learns talents as spells, so `Player::HasTalent` is false for them: core rules that check it (Leader of
  the Pack on shapeshift) need `HasSpell` too. Talents whose only effect is a dummy that "teaches" spells (Feral
  Charge 49377) need an `AscensionFreePickLinkedSpells.h` group, or they teach nothing.
- Old WotLK shapeshift rules in `HandleShapeshiftBoosts` cast extra auras for talents (Heart of the Wild,
  Survival of the Fittest) that the Ascension ranks already give through modifiers on the form passives; skip them
  under Free Pick. Talent rows without a `Talent.dbc` chain (Improved Travel Form) need `spell_ranks` rows, or every
  rank's aura stacks.
- `apps/coa-tests/talent_rank_restoration` reads `/data/dbc/CharacterAdvancement.dbc`; copy the owner's file there
  to run it.
- Ascension capstone bonuses usually live in hidden helpers (`<Talent> capstone sls`, `816xxx`/`857xxx` ids) that
  have no proc flags or nothing applying them; search by name and description before calling a capstone missing.

## Builds done (2026-10-28)

`compoundpower-build` (Marksman), `dragonwarrior-build` (Dragon Warrior), `bloodandguts-build` (Gutripper),
`knightoftheeclipse-build` (Knight of the Eclipse), `treeofwrath-build` (Tree of Wrath),
`masterofshadows-build` (Master of Shadows) and `evoker-build` (Evoker), with their follow-ups. Harnesses:
`area52_marksman`, `area52_dragon_warrior`, `area52_gutripper`, `area52_knight_of_the_eclipse`,
`area52_tree_of_wrath`, `area52_master_of_shadows`, `area52_evoker`.
