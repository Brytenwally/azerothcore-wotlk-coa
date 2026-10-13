# Map draft and feedback repair

New Draft now exits Brush/Trace mode, discards the previous server draft when present, and
ignores stale draft snapshots until the requested new center is acknowledged. Normal map
click processing is suppressed during GM CTRL editing, while ordinary map clicks retain
existing behavior. Saving brush geometry remains an explicit action.

Tool notifications and server OK/ERROR responses remain in chat and are also displayed in a
movable, scrollable XP Hotspot Messages window above the map. History is capped at 100 lines;
no unrelated chat messages are captured. The window uses the existing navy/gold UI style.

The Lua 5.1 map UI suite covers creation after brush mode, late old-draft responses, accepting
and editing the new draft, feedback history and layer placement, and native click forwarding.
Client-only change: no server restart is needed. Stage into the latest matching client archive
before release, preserving unrelated pending files. Not installed or released by this commit.

## Follow-up: creation acknowledgment

A reported Durotar failure displayed "Create or edit a draft first" from the follow-up radius
request. The prior client sent newmap and radius/point independently; the server silently
rejected any request arriving within 200 ms of the previous request, including requests
bunched together by network/server delays. The exact original packet trace was unavailable.

Creation now waits for a matching draft acknowledgment before setting radius or its first
polygon point. Additional map edits wait for that acknowledgment. The server permits a bounded
burst of eight requests per second and returns an explicit error when exhausted. Invalid map
creation also returns an error instead of disappearing. The client displays request context
and a response timeout. Tests cover Durotar identities and coordinates, deferred radius/point,
repeated acknowledgment, rapid clicks, same-timestamp bursts and rate-limit recovery.

This follow-up requires matching client/server activation; it is not active merely because
its source is published. In-game reproduction remains to be confirmed after installation.

## Area identity lookup

Durotar draft rejection was reproduced from effective AreaTable data: area 14 is on map 1 with exploration flag 127. The legacy helper looks up row 127, which is a subarea of Stranglethorn on map 0. The current area store uses the area ID as its index. Draft validation, saved-location validation and faction quotas now use direct area-ID lookup. Outdoor-map, parent-zone and coordinate restrictions remain in effect. Reviewed upstream: a65be06ec62f4a72002f5f73d61cca38d9f475da. In-game acceptance remains to be tested after activation.
