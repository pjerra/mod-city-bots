-- Adds bank POIs used by the city-roam hub alternation logic.
-- Clean installs get these rows from data/sql/db-world/updates/2026_07_13_01_city_bot_poi.sql.

INSERT INTO `city_bot_poi`
    (`id`, `location_key`, `zone_id`, `poi_type`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `district`, `weight`)
VALUES
    (26, 'Stormwind', 1519, 11, 0, -8918.0, 624.0, 99.5, 0.0, 'TradeDistrict', 120),
    (136, 'Orgrimmar', 1637, 11, 1, 1627.0, -4378.0, 16.0, 0.0, 'ValleyOfStrength', 120),
    (216, 'Ironforge', 1537, 11, 0, -4890.0, -985.0, 501.0, 0.0, 'Commons', 120),
    (316, 'Undercity', 1497, 11, 0, 1595.0, 230.0, -43.0, 0.0, 'MagicQuarter', 110),
    (716, 'Darnassus', 1657, 11, 1, 9865.0, 2325.0, 1315.0, 0.0, 'TradesmensTerrace', 110),
    (816, 'Exodar', 3557, 11, 530, -3985.0, -11730.0, -138.0, 0.0, 'CrystalHall', 110),
    (916, 'ThunderBluff', 1638, 11, 1, -1255.0, 125.0, 132.0, 0.0, 'LowerRise', 110),
    (1016, 'Silvermoon', 3487, 11, 530, 9640.0, -7490.0, 15.0, 0.0, 'Bazaar', 110),
    (1116, 'Shattrath', 3703, 11, 530, -1855.0, 5425.0, -12.0, 0.0, 'Terrace', 100),
    (1216, 'Dalaran', 4395, 11, 571, 5885.0, 608.0, 650.0, 0.0, 'MagusCommerce', 110)
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
