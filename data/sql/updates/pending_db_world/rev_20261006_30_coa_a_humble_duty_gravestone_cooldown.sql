-- A Humble Duty: upstream c683075584, qualified against the Area 52 goober reader.
-- Add a shared 30-second reuse cooldown to each grave and keep it from despawning after use.
-- This prevents rapid repeat clicks; it does not permanently track which graves each player used.
-- GameObject::Use checks GO_FLAG_IN_USE before credit and sets it when autoCloseTime is nonzero.
-- The timer clears the flag after 30000 milliseconds. Data5=0 preserves the static grave afterward.
UPDATE `gameobject_template` SET `Data3` = 30000, `Data5` = 0
WHERE `entry` IN (254677, 254678);
