-- Upstream 2983623a03; preserve the complete existing SmartAI block and creature statistics.
DELETE FROM `smart_scripts` WHERE `entryorguid`=7273 AND `source_type`=0;
INSERT INTO `smart_scripts` (`id`,`link`,`comment`,`target_o`,`target_x`,`target_y`,`target_z`,`event_type`,`action_type`,`entryorguid`,`event_flags`,`source_type`,`target_type`,`event_chance`,`event_param1`,`event_param2`,`event_param3`,`event_param4`,`event_param5`,`event_param6`,`action_param1`,`action_param2`,`action_param3`,`action_param4`,`action_param5`,`action_param6`,`target_param1`,`target_param2`,`target_param3`,`target_param4`,`event_phase_mask`) VALUES
(0,1,'Gahz''rilla - On AI Init - Move To Pos',0.0,1696.9100341796875,1216.5999755859375,8.880000114440918,37,69,7273,0,0,8,100,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0),
(1,0,'Gahz''rilla - On AI Init - Set Unit Flags',0.0,0.0,0.0,0.0,61,18,7273,512,0,1,100,0,0,0,0,0,0,768,0,0,0,0,0,0,0,0,0,0),
(2,0,'Gahz''rilla - Movement Inform - Remove Unit Flags',0.0,0.0,0.0,0.0,34,19,7273,512,0,1,100,8,1,0,0,0,0,768,0,0,0,0,0,0,0,0,0,0),
(3,0,'Gahz''rilla - In Combat - Cast Icicle',0.0,0.0,0.0,0.0,0,11,7273,16,0,5,100,3000,7000,7000,13000,0,0,11131,0,0,0,0,0,40,0,0,0,0),
(4,0,'Gahz''rilla - In Combat - Cast Freeze Solid',0.0,0.0,0.0,0.0,0,11,7273,16,0,5,100,0,16000,15000,25000,0,0,11836,0,0,0,0,0,30,0,0,0,0),
(5,0,'Gahz''rilla - In Combat - Cast Gahz''rilla Slam',0.0,0.0,0.0,0.0,0,11,7273,16,0,1,100,10000,16000,25000,35000,0,0,11902,2,0,0,0,0,0,0,0,0,0),
(6,0,'Gahz''rilla - On Death - Set Instance Data 1 to 3',0.0,0.0,0.0,0.0,6,34,7273,512,0,1,100,0,0,0,0,0,0,1,4,0,0,0,0,0,0,0,0,0),
(9000,0,'Gahz''rilla - Ascension kit - In Combat (first 6-8 s, then every 10-12 s) - Cast ''Icicle''',0.0,0.0,0.0,0.0,0,11,7273,0,0,5,100,6000,8000,10000,12000,0,0,11131,2,0,0,0,0,0,1,0,0,0),
(9001,0,'Gahz''rilla - Ascension kit - In Combat (first 8.65-10.65 s, then every 20-22 s) - Cast ''Freeze Solid''',0.0,0.0,0.0,0.0,0,11,7273,0,0,5,100,8650,10650,20000,22000,0,0,11836,0,0,0,0,0,0,1,0,0,0),
(9002,0,'Gahz''rilla - Ascension kit - In Combat (first 11.35-13.35 s, then every 15-17 s) - Cast ''Gahz''rilla Slam''',0.0,0.0,0.0,0.0,0,11,7273,0,0,1,100,11350,13350,15000,17000,0,0,11902,2,0,0,0,0,0,0,0,0,0);
UPDATE `creature_template` SET `ManaModifier`=2 WHERE `entry`=5710;
