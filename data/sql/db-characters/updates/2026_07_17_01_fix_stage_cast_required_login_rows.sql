-- mod-city-bots V2 - backfill required login rows for the full fixed stage cast.
--
-- Player::LoadFromDB hard-fails when character_homebind is missing. Earlier
-- seeds created the 9000001-9000400 characters but only part of the cast had
-- companion login rows, which made city-bot login stop at the same roster
-- boundary every run.

UPDATE `characters`
   SET `at_login` = 0,
       `equipmentCache` = ''
 WHERE `guid` >= 9000001
   AND `guid` < 9000401;

UPDATE `characters`
   SET `account` = 12001 + (`guid` - 9000001)
 WHERE `guid` >= 9000001
   AND `guid` < 9000401
   AND `account` <> 12001 + (`guid` - 9000001);

DELETE FROM `character_homebind`
 WHERE `guid` >= 9000001
   AND `guid` < 9000401;

INSERT INTO `character_homebind` (`guid`, `mapId`, `zoneId`, `posX`, `posY`, `posZ`)
SELECT `guid`, `map`, `zone`, `position_x`, `position_y`, `position_z`
  FROM `characters`
 WHERE `guid` >= 9000001
   AND `guid` < 9000401;
