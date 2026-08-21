-- mod-city-bots V2 - remove invalid one-size weapon seed and snap SW gate duelers to tightened POIs.

SET @CITYBOT_ITEM_BASE := 910000000;

DELETE FROM `mail_items`
 WHERE `item_guid` >= @CITYBOT_ITEM_BASE
   AND `item_guid` <  @CITYBOT_ITEM_BASE + 10000;

DELETE FROM `character_inventory`
 WHERE `item` >= @CITYBOT_ITEM_BASE
   AND `item` <  @CITYBOT_ITEM_BASE + 10000;

DELETE FROM `item_instance`
 WHERE `guid` >= @CITYBOT_ITEM_BASE
   AND `guid` <  @CITYBOT_ITEM_BASE + 10000;

UPDATE `characters` `c`
  JOIN `acore_playerbots`.`citizen_roster` `r` ON `r`.`guid` = `c`.`guid`
  JOIN `acore_world`.`city_bot_poi` `p` ON `p`.`id` = `r`.`poi_id`
   SET `c`.`map` = `p`.`map_id`,
       `c`.`zone` = `p`.`zone_id`,
       `c`.`position_x` = `p`.`pos_x`,
       `c`.`position_y` = `p`.`pos_y`,
       `c`.`position_z` = `p`.`pos_z`,
       `c`.`orientation` = `p`.`orientation`
 WHERE `r`.`home_key` = 'StormwindGate'
   AND `c`.`guid` >= 9000001
   AND `c`.`guid` < 9000401;
