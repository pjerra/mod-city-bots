-- mod-city-bots V2 - remap each stage-cast character to its own account

UPDATE `characters`
   SET `account` = 12001 + (`guid` - 9000001)
 WHERE `guid` >= 9000001
   AND `guid` < 9000401;
