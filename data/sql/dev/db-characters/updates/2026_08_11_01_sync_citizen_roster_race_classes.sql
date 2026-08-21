-- Fix invalid WotLK race/class pairs in the citizen roster ITSELF.
--
-- 2026_07_16_08_sync_stage_cast_race_classes.sql repaired the `characters`
-- rows but left `acore_playerbots`.`citizen_roster` carrying the same 19
-- invalid mage pairs (orc/dwarf/night-elf mage), so any later
-- roster -> characters identity sync (2026_07_25_02) reverts the repair and
-- the worldserver refuses to load those citizens again ("incorrect race/class
-- pair"). Fix the roster to the exact pairs 2026_07_16_08 assigned, then
-- re-sync the affected character rows so both stay in agreement.

UPDATE `acore_playerbots`.`citizen_roster` SET
  `race` = CASE `guid`
    WHEN 9000006 THEN 4
    WHEN 9000044 THEN 3
    WHEN 9000061 THEN 3
    WHEN 9000072 THEN 4
    WHEN 9000091 THEN 3
    WHEN 9000097 THEN 4
    WHEN 9000102 THEN 4
    WHEN 9000121 THEN 3
    WHEN 9000136 THEN 2
    WHEN 9000166 THEN 2
    WHEN 9000196 THEN 2
    WHEN 9000234 THEN 7
    WHEN 9000282 THEN 3
    WHEN 9000293 THEN 4
    WHEN 9000306 THEN 2
    WHEN 9000332 THEN 3
    WHEN 9000352 THEN 3
    WHEN 9000371 THEN 2
    WHEN 9000391 THEN 2
    ELSE `race` END,
  `class` = CASE `guid`
    WHEN 9000006 THEN 11
    WHEN 9000044 THEN 3
    WHEN 9000061 THEN 3
    WHEN 9000072 THEN 11
    WHEN 9000091 THEN 3
    WHEN 9000097 THEN 11
    WHEN 9000102 THEN 1
    WHEN 9000121 THEN 3
    WHEN 9000136 THEN 3
    WHEN 9000166 THEN 3
    WHEN 9000196 THEN 1
    WHEN 9000234 THEN 4
    WHEN 9000282 THEN 4
    WHEN 9000293 THEN 1
    WHEN 9000306 THEN 3
    WHEN 9000332 THEN 4
    WHEN 9000352 THEN 1
    WHEN 9000371 THEN 3
    WHEN 9000391 THEN 3
    ELSE `class` END
WHERE `guid` IN (9000006, 9000044, 9000061, 9000072, 9000091, 9000097,
                 9000102, 9000121, 9000136, 9000166, 9000196, 9000234,
                 9000282, 9000293, 9000306, 9000332, 9000352, 9000371,
                 9000391);

UPDATE `characters` `c`
  JOIN `acore_playerbots`.`citizen_roster` `r` ON `r`.`guid` = `c`.`guid`
   SET `c`.`race` = `r`.`race`,
       `c`.`class` = `r`.`class`,
       `c`.`at_login` = 0,
       `c`.`equipmentCache` = ''
 WHERE `c`.`guid` IN (9000006, 9000044, 9000061, 9000072, 9000091, 9000097,
                      9000102, 9000121, 9000136, 9000166, 9000196, 9000234,
                      9000282, 9000293, 9000306, 9000332, 9000352, 9000371,
                      9000391);
