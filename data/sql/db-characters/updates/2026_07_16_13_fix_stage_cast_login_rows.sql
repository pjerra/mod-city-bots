-- mod-city-bots V2 - fix stage-cast rows that fail Player::LoadFromDB on bot login.
-- Symptoms: AddPlayerBot queues OK but HandlePlayerLoginFromDB returns null for
-- flex citizens (GsCitizen*, SwCitizen*, OrCitizen*) and some gate bots (Orgrim).
-- Causes: stale at_login flags, character rows drifting from citizen_roster,
-- or corrupt equipmentCache from old outfit seeds.

UPDATE `characters`
   SET `at_login` = 0,
       `online` = 0,
       `equipmentCache` = ''
 WHERE `guid` >= 9000001
   AND `guid` < 9000401;

UPDATE `characters` `c`
  JOIN `acore_playerbots`.`citizen_roster` `r` ON `r`.`guid` = `c`.`guid`
   SET `c`.`account` = `r`.`account_id`,
       `c`.`name` = `r`.`character_name`,
       `c`.`race` = `r`.`race`,
       `c`.`class` = `r`.`class`,
       `c`.`gender` = `r`.`gender`,
       `c`.`level` = `r`.`level`,
       `c`.`at_login` = 0,
       `c`.`online` = 0,
       `c`.`equipmentCache` = ''
 WHERE `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401
   AND `r`.`enabled` = 1;

-- Keep fixed Goldshire duelers and dedicated fishers on safe, current POIs.
UPDATE `characters` `c`
  JOIN `acore_playerbots`.`citizen_roster` `r` ON `r`.`guid` = `c`.`guid`
  JOIN `acore_world`.`city_bot_poi` `p` ON `p`.`id` = `r`.`poi_id`
   SET `c`.`map` = `p`.`map_id`,
       `c`.`zone` = `p`.`zone_id`,
       `c`.`position_x` = `p`.`pos_x`,
       `c`.`position_y` = `p`.`pos_y`,
       `c`.`position_z` = `p`.`pos_z`,
       `c`.`orientation` = `p`.`orientation`,
       `c`.`at_login` = 0,
       `c`.`online` = 0
 WHERE `c`.`guid` IN (9000027, 9000028, 9000048, 9000049,
                      9000050, 9000051, 9000126, 9000127,
                      9000271, 9000272)
   AND `r`.`enabled` = 1;
