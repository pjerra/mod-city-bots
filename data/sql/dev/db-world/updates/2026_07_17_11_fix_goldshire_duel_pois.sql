-- Move Goldshire dedicated duel points off the inn porch/stairs so Denton/Rylee
-- can see and reach each other. Some dev DBs are missing these POIs entirely.
INSERT INTO `city_bot_poi`
    (`id`, `location_key`, `zone_id`, `poi_type`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `district`, `weight`)
VALUES
    (408, 'Goldshire', 12, 9, 0, -9452.0, 89.0, 56.0, 5.7, 'Center', 100),
    (409, 'Goldshire', 12, 9, 0, -9476.0, 94.0, 56.0, 3.8, 'Center', 100)
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
