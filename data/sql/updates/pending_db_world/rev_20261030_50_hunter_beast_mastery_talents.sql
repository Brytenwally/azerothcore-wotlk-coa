-- Cobra Strikes: critical Arcane Shot, Steady Shot, Raptor Strike, Mongoose Bite, Frenzy Shot or Counterattack
-- hits, melee ones included (Kill Shot is no longer on the tooltip)
-- Invigoration rank 3 capstone: casting Intimidation heals the pet for 20% (853255)
DELETE FROM `spell_proc` WHERE `SpellId` IN (-53256, 853253);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(-53256, 0, 9, 2048, 524289, 37814272, 69968, 1, 2, 2, 0, 0, 0, 0, 0, 0),
(853253, 0, 9, 0, 0, 8, 81920, 7, 1, 0, 0, 1, 0, 0, 0, 0);
