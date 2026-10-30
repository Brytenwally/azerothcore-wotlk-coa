-- Rapid Killing: the buff empowers the next two Aimed, Arcane, Kill or Chimera Shots (owner, 30/10)
-- Wild Quiver: Steady Shot procs it too, as the tooltip says
-- Piercing Shots: Arcane Shot, Multi-Shot and Kill Shot crits too, as the tooltip says
DELETE FROM `spell_proc` WHERE `SpellId` IN (35098, 35099, 835099, -53215, -53234);
INSERT INTO `spell_proc` (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
`SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
`ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`) VALUES
(35098, 0, 0, 0, 0, 0, 0, 1, 2, 0, 8, 7, 0, 0, 0, 2),
(35099, 0, 0, 0, 0, 0, 0, 1, 2, 0, 8, 7, 0, 0, 0, 2),
(835099, 0, 0, 0, 0, 0, 0, 1, 2, 0, 8, 7, 0, 0, 0, 2),
(-53215, 0, 9, 1, 1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0),
(-53234, 0, 9, 137216, 8388609, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0);

-- Careful Aim, Rapid Killing (Rapid Fire cooldown) and Piercing Shots (healing and absorb reduction) grant their
-- per-rank helpers; Mortal Shots lengthens Aimed Shot by 1/2/3 sec; Improved Trueshot Aura follows Trueshot Aura;
-- Improved Steady Shot's and Improved Stings' capstones
DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('aura_area52_hunter_rank_grant', 'aura_area52_mortal_shots',
'aura_area52_improved_trueshot_aura', 'aura_area52_improved_steady_shot', 'aura_area52_capstone_steady_shot_buff',
'aura_area52_serpent_sting_dispel');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(34482, 'aura_area52_hunter_rank_grant'),
(34483, 'aura_area52_hunter_rank_grant'),
(34484, 'aura_area52_hunter_rank_grant'),
(34948, 'aura_area52_hunter_rank_grant'),
(34949, 'aura_area52_hunter_rank_grant'),
(834949, 'aura_area52_hunter_rank_grant'),
(53234, 'aura_area52_hunter_rank_grant'),
(53237, 'aura_area52_hunter_rank_grant'),
(53238, 'aura_area52_hunter_rank_grant'),
(19485, 'aura_area52_mortal_shots'),
(19487, 'aura_area52_mortal_shots'),
(19488, 'aura_area52_mortal_shots'),
(435104, 'aura_area52_improved_trueshot_aura'),
(435105, 'aura_area52_improved_trueshot_aura'),
(-53221, 'aura_area52_improved_steady_shot'),
(853220, 'aura_area52_capstone_steady_shot_buff'),
(-1978, 'aura_area52_serpent_sting_dispel');
