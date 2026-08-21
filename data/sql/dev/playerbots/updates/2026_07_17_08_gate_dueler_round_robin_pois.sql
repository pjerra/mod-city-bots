-- mod-city-bots V2 dev patch - spread gate duelers across six duel POIs.
--
-- This is for already-running dev databases. The same final state is folded
-- into playerbots/updates/2026_07_15_00_citizen_roster.sql for clean installs.

UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 6)
       WHEN 0 THEN 501
       WHEN 1 THEN 502
       WHEN 2 THEN 503
       WHEN 3 THEN 504
       WHEN 4 THEN 505
       ELSE 506
   END
 WHERE `home_key` = 'StormwindGate'
   AND `role` = 3;

UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 6)
       WHEN 0 THEN 601
       WHEN 1 THEN 602
       WHEN 2 THEN 603
       WHEN 3 THEN 604
       WHEN 4 THEN 605
       ELSE 606
   END
 WHERE `home_key` = 'OrgrimmarGate'
   AND `role` = 3;
