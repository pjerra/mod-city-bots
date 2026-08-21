-- Hotfix: keep stage-cast character identities exactly aligned with citizen_roster.
-- Apply after any playerbots citizen_roster identity/name/account change.

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
