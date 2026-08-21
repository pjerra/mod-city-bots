-- mod-city-bots uninstall (acore_playerbots)
-- Run manually when removing the module. Not applied by the worldserver updater.
--
-- Stop worldserver first. After this script, restart worldserver so playerbots
-- can run AssignAccountTypes() and refill the random-bot account pool.
--
-- mysql -u ... -p acore_playerbots < uninstall_mod_city_bots.sql

DELETE FROM `playerbots_random_bots`
 WHERE `bot` BETWEEN 9000001 AND 9000400
   AND `event` IN ('add', 'logout', 'citizen', 'citizen_burst', 'citizen_home',
                   'randomize', 'teleport', 'update');

UPDATE `playerbots_account_type`
   SET `account_type` = 0
 WHERE `account_id` BETWEEN 12001 AND 12400
   AND `account_type` = 3;

DROP TABLE IF EXISTS `citizen_roster`;
