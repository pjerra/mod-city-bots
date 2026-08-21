-- mod-city-bots V2 - restore persistent basic clothing for stage-cast ambience.
-- Inn dancers remain intentionally unclothed. Duelers are upgraded at runtime
-- by the module at runtime, but these starter clothes prevent naked first-loads.

SET @CITYBOT_ITEM_BASE := 910100000;

DROP TEMPORARY TABLE IF EXISTS `tmp_citybot_basic_outfit`;
CREATE TEMPORARY TABLE `tmp_citybot_basic_outfit` (
  `slot` TINYINT UNSIGNED NOT NULL,
  `itemEntry` INT UNSIGNED NOT NULL,
  `offset` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`slot`)
);

INSERT INTO `tmp_citybot_basic_outfit` (`slot`, `itemEntry`, `offset`) VALUES
  (3, 38, 1),  -- Recruit's Shirt
  (6, 39, 2),  -- Recruit's Pants
  (7, 40, 3);  -- Recruit's Boots

DELETE FROM `character_inventory`
 WHERE `item` >= @CITYBOT_ITEM_BASE
   AND `item` <  @CITYBOT_ITEM_BASE + 10000;

DELETE FROM `item_instance`
 WHERE `guid` >= @CITYBOT_ITEM_BASE
   AND `guid` <  @CITYBOT_ITEM_BASE + 10000;

INSERT INTO `item_instance`
    (`guid`, `itemEntry`, `owner_guid`, `creatorGuid`, `giftCreatorGuid`,
     `count`, `duration`, `charges`, `flags`, `enchantments`,
     `randomPropertyId`, `durability`, `playedTime`, `text`)
SELECT @CITYBOT_ITEM_BASE + ((`r`.`guid` - 9000001) * 10) + `oi`.`offset`,
       `oi`.`itemEntry`,
       `r`.`guid`,
       0,
       0,
       1,
       0,
       '0 0 0 0 0 ',
       0,
       '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 ',
       0,
       20,
       0,
       ''
  FROM `acore_playerbots`.`citizen_roster` `r`
 CROSS JOIN `tmp_citybot_basic_outfit` `oi`
 WHERE `r`.`guid` >= 9000001
   AND `r`.`guid` < 9000401
   AND `r`.`enabled` = 1
   AND `r`.`role` <> 1
   AND NOT EXISTS (
     SELECT 1
       FROM `character_inventory` `ci`
      WHERE `ci`.`guid` = `r`.`guid`
        AND `ci`.`bag` = 0
        AND `ci`.`slot` = `oi`.`slot`
   );

INSERT INTO `character_inventory` (`guid`, `bag`, `slot`, `item`)
SELECT `r`.`guid`,
       0,
       `oi`.`slot`,
       @CITYBOT_ITEM_BASE + ((`r`.`guid` - 9000001) * 10) + `oi`.`offset`
  FROM `acore_playerbots`.`citizen_roster` `r`
 CROSS JOIN `tmp_citybot_basic_outfit` `oi`
 WHERE `r`.`guid` >= 9000001
   AND `r`.`guid` < 9000401
   AND `r`.`enabled` = 1
   AND `r`.`role` <> 1
   AND NOT EXISTS (
     SELECT 1
       FROM `character_inventory` `ci`
      WHERE `ci`.`guid` = `r`.`guid`
        AND `ci`.`bag` = 0
        AND `ci`.`slot` = `oi`.`slot`
   );

DROP TEMPORARY TABLE IF EXISTS `tmp_citybot_basic_outfit`;
