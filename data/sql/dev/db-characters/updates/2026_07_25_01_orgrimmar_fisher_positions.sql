-- Move the two dedicated Orgrimmar fishers to their fishable-water POIs.
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
 WHERE `c`.`guid` IN (9000126, 9000127)
   AND `r`.`enabled` = 1;
