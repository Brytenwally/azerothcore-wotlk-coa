# Area 52 Feed Pet retirement

This deployment patch targets the current Area 52 reconstruction, whose `AscensionHeroPets`
and `AscensionFreePick` implementations differ from upstream main. Do not apply it to
upstream Wildcard. Reviewed upstream baseline: 4f66bddb12a32eeba49d42621651f19c46c4f570.

Apply `patches/area52/feed-pet-removal.patch` to the matching Area 52 source before building.
It removes 6991 from both Beast grant lists, denies future learning of Feed Pet variants
6991/1106991/1356991 on Free Pick heroes, and unlearns existing copies across all specs on
login or Beast package reconciliation. Action-bar cleanup also handles orphaned buttons.
Other pet spells, including Mend Pet, remain unchanged. Realm/class gating is retained.

The matching client overlay removes Feed Pet from spell 965200's English description
and removes the root-6991 row from Area52MysticRules/GrantedAbilityCatalog.lua. Do not remove
shared Feed Pet spell definitions. Client archives and extracted DBCs are deployment artifacts,
not source files. Preserve all other records, strings, catalog rows and archive members.

The Area 52 integration build passed via tools/verify_all.py (build stage). In-game acceptance
is pending. Server source, binary, tooltip archive and addon catalog are staged in
outputs/Area52_Feed_Pet_Removal in the owner workspace. No live deployment or restart occurred.
The generic Feed Pet healing correction remains independent on fix/feed-pet-healing.
