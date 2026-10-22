-- Upstream 413e03e986 and 2bf5d4a206: preserve all other eligibility and quest fields.
UPDATE `quest_template` SET `AllowableRaces` = 690 WHERE `ID` IN (8325,8326,8327,8330,8334,8335,8336,8338,8345,8347) AND `AllowableRaces` = 512;
UPDATE `quest_template` SET `AllowableRaces` = 690 WHERE `ID` = 4134;
