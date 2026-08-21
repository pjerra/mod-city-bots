-- Runtime support POIs for the duel recovery/fishing pass.
-- Includes safer Goldshire duel points, the requested Goldshire pond spots,
-- and second fishable-water points for Stormwind, Orgrimmar, and Ironforge.

INSERT INTO `city_bot_poi`
    (`id`, `location_key`, `zone_id`, `poi_type`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `district`, `weight`)
VALUES
    (11,  'Stormwind', 1519, 5, 0, -8426.0, 1325.0, 5.0, 0.0, 'Harbor', 100),
    (107, 'Orgrimmar', 1637, 5, 1, 1991.7499, -4659.4850, 27.375679, 3.8521101, 'ValleyOfHonor', 100),
    (119, 'Orgrimmar', 1637, 5, 1, 1506.4592, -4188.6763, 41.288647, 2.3166604, 'ValleyOfSpirits', 100),
    (205, 'Ironforge', 1537, 5, 0, -4638.0, -1114.0, 501.0, 0.0, 'ForlornCavern', 100),
    (206, 'Ironforge', 1537, 5, 0, -4644.0, -1119.0, 501.0, 0.0, 'ForlornCavern', 100),
    (416, 'Goldshire', 12, 5, 0, -9449.83, 144.70833, 56.253647, 1.1336792, 'CrystalLake', 100),
    (417, 'Goldshire', 12, 5, 0, -9449.54, 145.1067, 56.253647, 1.1336792, 'CrystalLake', 100)
ON DUPLICATE KEY UPDATE
    `location_key` = VALUES(`location_key`),
    `zone_id` = VALUES(`zone_id`),
    `poi_type` = VALUES(`poi_type`),
    `map_id` = VALUES(`map_id`),
    `pos_x` = VALUES(`pos_x`),
    `pos_y` = VALUES(`pos_y`),
    `pos_z` = VALUES(`pos_z`),
    `orientation` = VALUES(`orientation`),
    `district` = VALUES(`district`),
    `weight` = VALUES(`weight`);

UPDATE `city_bot_poi`
   SET `pos_x` = -9450.0,
       `pos_y` = 72.0,
       `pos_z` = 56.0,
       `orientation` = 5.7,
       `district` = 'Center'
 WHERE `id` = 408;

UPDATE `city_bot_poi`
   SET `pos_x` = -9469.0,
       `pos_y` = 78.0,
       `pos_z` = 56.0,
       `orientation` = 3.8,
       `district` = 'Center'
 WHERE `id` = 409;
