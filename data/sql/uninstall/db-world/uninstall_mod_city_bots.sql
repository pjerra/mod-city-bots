-- mod-city-bots uninstall (acore_world)
-- Run manually when removing the module. Not applied by the worldserver updater.
--
-- mysql -u ... -p acore_world < uninstall_mod_city_bots.sql

-- POI data (module-owned table)
DROP TABLE IF EXISTS `city_bot_poi`;

-- Optional guard ambiance SmartAI (mod-city-bots comments, ids 1-4 per entry)
DELETE FROM `smart_scripts`
 WHERE `source_type` = 0
   AND `entryorguid` IN (68, 1976, 3296)
   AND `id` BETWEEN 1 AND 4
   AND `comment` LIKE 'mod-city-bots:%';

-- Human/undead hunter compatibility rows owned by this module.
DELETE FROM `playercreateinfo_action` WHERE `race` IN (1, 5) AND `class` = 3;
DELETE FROM `playercreateinfo` WHERE `race` IN (1, 5) AND `class` = 3;
