-- Snap Denton/Rylee and dedicated fishers to their current safe POIs.
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
