-- Legacy compatibility: stage-cast names are now seeded as readable, valid player names.
-- Keep character identity aligned with citizen_roster instead of rewriting to short code names.

UPDATE `characters` `c`
  JOIN `acore_playerbots`.`citizen_roster` `r` ON `r`.`guid` = `c`.`guid`
   SET `c`.`name` = `r`.`character_name`,
       `c`.`at_login` = 0,
       `c`.`online` = 0
 WHERE `c`.`guid` BETWEEN 9000001 AND 9000400;
