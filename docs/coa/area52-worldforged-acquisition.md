# Area 52 Worldforged acquisition

Current construction status: see the [16-cache batch](area52-worldforged-batch16.md).
The original Flameburst-only SQL described below was superseded before activation.

This package is for the Area 52 reconstruction, not a universal import for all realms.
It does not change enchant verification status or claim original live-server behavior was tested.

## Reviewed sources

- CoA main: `4f66bddb12a32eeba49d42621651f19c46c4f570`.
- [Area 52 acquisition reference](https://ascension.help/), archived by
  [Open Ascension](https://github.com/synthalorian/open_ascension/tree/54fe0932b1dde222960a889b1d4d263976d6349d/assets/data).
  Server ID `9ee7b1a0-2cd6-44ca-881f-56aff5cf9f56` is explicitly Area 52.
  Its records are community references labelled unverified, not authoritative server captures.
- [Hertig cache](https://github.com/hertigservices/ascension-data/tree/32ef526ca12acc2d10d6696750f0ee0190814f25/supplemental/worldforged).
  The Bisbeard atlas mixes modes. Name matches alone do not establish a Free Pick location.
- Effective local Area 52 MysticEnchant.dbc: require Free Pick realm field 8, HERO mask 512,
  and Worldforged field 7, then match the identified scroll catalog.

The initial index contains 176 Area 52 references; 173 match currently eligible Worldforged scrolls.
43 descriptions flag interactions requiring further reconstruction. These are triage counts,
not counts of validated placements. The accompanying audit preserves per-enchant source links.

## First staged cache: Flameburst

Activation hold: the owner-confirmed key contract in [the plan](area52-worldforged-plan.md)
supersedes the prototype's inherited once-per-character gear pickup behavior. Replace its handler
with key-consuming scroll acquisition before deployment; the current SQL is only a location prototype.

[Area 52 record](https://ascension.help/mystic/e944c2dc-0f8c-4b41-87b3-041a829b5fee)
places a cache near Jergosh in Ragefire Chasm. The upstream pickup migration independently has
object 1345173 at map 389, -371.200 / 191.000 / -21.130, facing 0.624764.
Reuse its spawn identity 6910429 and existing 100% reward item 100369, mapped to enchant spell 92659.
The existing `worldforged_pickup` implementation supplies persistent per-character claims.
The identified-scroll handler supplies the existing collection confirmation; looting does not teach it.

The room/location is corroborated, but the upstream height was inferred from a nearby object.
This is a staged test placement, not a verified original position. Check floor height, reachability,
loot, full bags, repeat use, a second character, and instance reset before general release.
No drop probability is invented for Jergosh. Existing creature loot is unchanged.

The SQL inserts only if the expected Area 52 item handler and cache bindings exist and both the
spawn identity and a cache in the instance are absent. Reapplication replaces only this package's
own marked row. It does not alter unrelated or upstream-owned spawns.
This also makes reapplication preserve claims rather than mint another pickup identity.

## Deferred rather than simplified

- Earthborer Acid: CoA cache provenance names Deadmines; Area 52 names Ragefire Chasm.
- Maggot Trap: Area 52 identifies a Stratholme supply crate, but not a unique crate position.
- Disciple of Arugal: reconcile the recovered point with the specified Silverlaine hall stage.
- Quest chains, timed books, keys, reputation/vendor gates, rare spawns and conditional kills:
  retain their acquisition steps. Do not replace these with free ground pickups.

No server restart, client release or upstream PR is included in this staging milestone.
