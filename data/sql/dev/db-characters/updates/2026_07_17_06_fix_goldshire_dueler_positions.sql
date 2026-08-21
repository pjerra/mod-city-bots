-- Snap Denton/Rylee out of the inn geometry and make them valid level-80 duelers.
UPDATE `characters`
   SET `map` = 0,
       `zone` = 12,
       `level` = 80,
       `position_x` = -9452.0,
       `position_y` = 89.0,
       `position_z` = 56.0,
       `orientation` = 5.7
 WHERE `guid` = 9000027;

UPDATE `characters`
   SET `map` = 0,
       `zone` = 12,
       `level` = 80,
       `position_x` = -9476.0,
       `position_y` = 94.0,
       `position_z` = 56.0,
       `orientation` = 3.8
 WHERE `guid` = 9000028;
