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
