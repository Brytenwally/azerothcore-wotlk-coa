# Demonic Leap visual repair

Owner authorized adding the confirmed wings-link repair to the pending package; preparation only. Preserve all blessing and replacement-interaction changes. No gameplay, activation or client installation.

## Confirmed chain and repair

Demonic Leap954521 uses base visual124974 and had no trigger to wings helper954543. Effective SQL has only the existing Leap script binding, no linked spell or spell_dbc override. Untouched client and installed client agree on helper954543→visual21888→state kit21827→model attachment5711→effect154242→SPELLS/spell_warlockmorphwings.m2. The model is already installed in patch-N; no client asset, DLL or launcher-feed change is needed.

The existing Area52-only spell_area52_demonic_leap script now validates helper954543 and casts it on EFFECT_2/SPELL_EFFECT_DUMMY hit-target. That slot targets the caster in the effective record. It is reached on a successful hit, not at cast initiation; the helper is not bound back to the Leap script. It runs on both the first leap and reactivation. Existing Before/After cooldown logic is untouched. No jump parameters or movement implementation change.

Helper954543 has one self dummy aura, no damage, phase or power effects. Native duration record28 is5000ms; Leap's reactivation aura remains duration36=1000ms. Repeated helper casts use existing same-spell aura refresh; ordinary finite-aura expiration removes it. No persistent custom state or timer is introduced. Do not substitute3304522: its similar name hides unrelated mechanics.

## Offline evidence and limits

Extracted complete native Leap script is exercised through mocked script/player adapters:16 combinations of Area52/other-mode, player/nonplayer, first/reactivated, hit/no-hit; an actual first/reactivated handler sequence; missing helper validation; and registered effect slot/type. Tests confirm one self-targeted triggered helper cast per successful hit and unchanged cooldown removals. Data checks confirm5000ms/1000ms durations. Existing core aura refresh/expiration is reused, not newly implemented or claimed simulated by this harness. Actual rendered wings and gameplay remain tester acceptance.

The combined runner retains all576 replacement cases,4356 blessing decisions, native stronger/weaker aura checks and32 publication/deployment fixtures. Full incremental compilation and unit checks passed; source/style and stable package preflight are separate gates. No new SQL, status, tooltip or certification changes.

## Preserved prior package audit

# Paladin blessing stacking correction

Owner confirmed that different Paladin blessings must not remove each other; higher ranks may replace lower ranks of the same blessing. Sap was separately confirmed working and is closed without a source change.

## Cause and minimal correction

Effective spell_group_stack_rules row1010 applies EXCLUSIVE_FROM_SAME_CASTER to all six blessing subgroups. This explains why Might, Wisdom, Kings and Sanctuary replace each other from one caster. Their individual groups1002/1005/1006/1007 already handle duplicate copies and strongest effects. Native CheckSpellGroupStackRules selects the narrower subgroup when both spells share it.

The loader changes only group1010's existing EXCLUSIVE_FROM_SAME_CASTER rule to DEFAULT while AscensionFreePick::Enabled() is true. No database row is changed. Other rule values and other realm modes remain unchanged. The same loader path covers startup and group reload. Individual blessing groups and native rank exclusivity remain intact. No blanket allow-stack shortcut bypasses other aura restrictions.

## Qualification

fixture.json captures actual group membership, rules and all33 Might/Wisdom/Kings/Sanctuary rank records read-only. The runner extracts production CheckSpellGroupStackRules and the exact loader correction. 4,356 pairwise native decisions cover all ranks in both orders, baseline versus correction, and Area52 versus other modes. The baseline reproduces cross-blessing exclusion; the correction yields DEFAULT only for different blessings. Same-blessing highest/exclusive rules remain unchanged, including normal/greater variants. Separate extracted Unit::IsHighestExclusiveAuraEffect checks reject weaker Might/Wisdom, remove weaker existing copies for stronger applications, and retain different blessings. Database and engine adapters are mocked; this is offline engineering evidence, not gameplay certification.

Build reused the qualified pending candidate cache and rebuilt only SpellMgr.cpp plus linking. Four existing compiler warnings concern unrelated large group IDs and their enum range; this three-line correction introduces none. Source and C++ style checks are separate gates. The previous five-file replacement candidate is retained in full. No SQL, client, tooltip or status changes; no activation/publication.

## Preserved replacement interaction scope

# Replacement interaction follow-up

Prepared scope: extend the pending shared replacement package with four eligibility paths in Area52HopliteRemaining.inc. Retain every source file from the prior candidate. Nothing is activated or published during preparation.

## Confirmed gaps and correction

Wind Slam's custom family filter bypassed the shared central proc-family fallback. Its consuming-ability list and shield slow decision also used only the event spell's own root. Draconic Sinister explicitly allowed Sinister Strike and Thrust but no subsequent authorized replacement. Each now checks the actual spell first, then active origins from Player::GetSpellInteractionOrigins. The predicate is evaluated as a boolean; several qualifying ancestors cannot duplicate a proc.

The adapter receives the actual event actor, established by the unchanged actor-equals-owner check. Wind Slam's handler uses that validated owner. Missing records and non-player actors cannot acquire ancestry. Current grants and Area 52 isolation come from the shared lookup. Direct existing eligibility, including explicit Thrust and Hoplite Shield Strike support, remains unchanged.

Preserved conditions: nonzero damage, nonperiodic events, aura ownership, active Wind Slam enchant, Draconic cooldown and random roll. Wind Slam casts once, applies its existing -20 shield slow only to shield consumers, and removes its buff once. Its normal non-shield consumer still has no slow. No new damage, coefficient, resource, ownership, SQL or status behavior is introduced.

## Offline evidence

The focused runner extracts the native shared graph, new adapter, both Check bodies and Wind Slam Handle body. Its matrix covers 128 Wind consumer cases and 32 Draconic cases, plus family replacement, missing inputs, direct rank handling, removal and slow behavior. The identical matrix reproduces the pre-migration Wind Slam failure. Existing 416 lookup/Tides/modifier/proc/consumer cases are retained. Engine adapters are mocked; these are engineering checks, not gameplay certification.

Initial fixture attempts were superseded: one mixed enum/unsigned initializer failed compilation; another incorrectly assumed Draconic's existing direct Thrust eligibility required an active Sinister Strike link. The final fixture correctly removes the immediate Thrust grant to test revoked ancestry. Neither required a source correction.

## Deliberately retained identities and remaining audit queue

Original ability acquisition, rank refresh and replacement grant operations remain exact identity checks. Other-class scripts (such as dedicated Pyromancer and Stormbringer mechanics) require realm/class reachability review before migration. Context-free AuraEffect predicates require a demonstrated actor at each caller.

The prior inventory is not an exhaustive migration claim. Further custom-script work remains, including White Walker's Frozen Blade pre-cast and result classification, Ice Lance critical trigger, and explicit Ice Spear/Winds of Winter interaction lists. Those need paired event/snapshot checks and must not be mixed into an already frozen build. This package is a qualified incremental migration, not a universal compatibility or whole-enchant Verified claim.

Production wiring: effective AscensionMysticEnchant.cpp includes Area52HopliteRemaining.inc at line 66 and registers aura_area52_wind_slam and aura_area52_draconic_sinister at lines 1201/1203. Registration and bindings are unchanged; complete compilation is required separately.
