-- mod-city-bots V2 - backfill stage-cast skill/reputation support rows.
--
-- The fixed cast had all 400 characters/homebinds, but only the first seeded
-- performers had character_skills/character_reputation rows. Playerbot login
-- consistently stopped at that roster boundary. Copy known-good rows from
-- seeded stage-cast donors to the rest of the dedicated cast.

DROP TEMPORARY TABLE IF EXISTS `city_bot_skill_donor_race_class`;
CREATE TEMPORARY TABLE `city_bot_skill_donor_race_class` AS
SELECT `c`.`race`, `c`.`class`, MIN(`c`.`guid`) AS `donor_guid`
  FROM `characters` `c`
  JOIN `character_skills` `s` ON `s`.`guid` = `c`.`guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401
 GROUP BY `c`.`race`, `c`.`class`;

DROP TEMPORARY TABLE IF EXISTS `city_bot_skill_donor_class`;
CREATE TEMPORARY TABLE `city_bot_skill_donor_class` AS
SELECT `c`.`class`, MIN(`c`.`guid`) AS `donor_guid`
  FROM `characters` `c`
  JOIN `character_skills` `s` ON `s`.`guid` = `c`.`guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401
 GROUP BY `c`.`class`;

INSERT IGNORE INTO `character_skills` (`guid`, `skill`, `value`, `max`)
SELECT `c`.`guid`, `s`.`skill`, `s`.`value`, `s`.`max`
  FROM `characters` `c`
  JOIN `city_bot_skill_donor_race_class` `d`
    ON `d`.`race` = `c`.`race`
   AND `d`.`class` = `c`.`class`
  JOIN `character_skills` `s` ON `s`.`guid` = `d`.`donor_guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;

INSERT IGNORE INTO `character_skills` (`guid`, `skill`, `value`, `max`)
SELECT `c`.`guid`, `s`.`skill`, `s`.`value`, `s`.`max`
  FROM `characters` `c`
  JOIN `city_bot_skill_donor_class` `d`
    ON `d`.`class` = `c`.`class`
  JOIN `character_skills` `s` ON `s`.`guid` = `d`.`donor_guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;

DROP TEMPORARY TABLE IF EXISTS `city_bot_rep_donor_race`;
CREATE TEMPORARY TABLE `city_bot_rep_donor_race` AS
SELECT `c`.`race`, MIN(`c`.`guid`) AS `donor_guid`
  FROM `characters` `c`
  JOIN `character_reputation` `r` ON `r`.`guid` = `c`.`guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401
 GROUP BY `c`.`race`;

DROP TEMPORARY TABLE IF EXISTS `city_bot_rep_donor_any`;
CREATE TEMPORARY TABLE `city_bot_rep_donor_any` AS
SELECT MIN(`c`.`guid`) AS `donor_guid`
  FROM `characters` `c`
  JOIN `character_reputation` `r` ON `r`.`guid` = `c`.`guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;

INSERT IGNORE INTO `character_reputation` (`guid`, `faction`, `standing`, `flags`)
SELECT `c`.`guid`, `r`.`faction`, `r`.`standing`, `r`.`flags`
  FROM `characters` `c`
  JOIN `city_bot_rep_donor_race` `d`
    ON `d`.`race` = `c`.`race`
  JOIN `character_reputation` `r` ON `r`.`guid` = `d`.`donor_guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;

INSERT IGNORE INTO `character_reputation` (`guid`, `faction`, `standing`, `flags`)
SELECT `c`.`guid`, `r`.`faction`, `r`.`standing`, `r`.`flags`
  FROM `characters` `c`
  JOIN `city_bot_rep_donor_any` `d`
  JOIN `character_reputation` `r` ON `r`.`guid` = `d`.`donor_guid`
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;

DROP TEMPORARY TABLE IF EXISTS `city_bot_skill_donor_race_class`;
DROP TEMPORARY TABLE IF EXISTS `city_bot_skill_donor_class`;
DROP TEMPORARY TABLE IF EXISTS `city_bot_rep_donor_race`;
DROP TEMPORARY TABLE IF EXISTS `city_bot_rep_donor_any`;
