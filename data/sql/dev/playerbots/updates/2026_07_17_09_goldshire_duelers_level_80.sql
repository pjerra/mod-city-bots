-- Goldshire duelers use the same level-80 duel rules as gate duelers.
UPDATE `citizen_roster`
   SET `level` = 80
 WHERE `guid` IN (9000027, 9000028)
   AND `role` = 4;
