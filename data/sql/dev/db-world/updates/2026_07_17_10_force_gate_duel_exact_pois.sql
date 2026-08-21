-- mod-city-bots V2 dev patch - force gate duel POIs to the tested locations.
--
-- This is for already-running dev databases. The same final state is folded
-- into db-world/updates/2026_07_17_09_gate_duel_points_and_city_roam_pois.sql
-- for clean installs.

INSERT INTO `city_bot_poi`
    (`id`, `location_key`, `zone_id`, `poi_type`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `district`, `weight`)
VALUES
    (501, 'StormwindGate', 12, 9, 0, -9126.377, 351.36536, 93.62221, 0.5799463, 'GateDuelRing', 100),
    (502, 'StormwindGate', 12, 9, 0, -9108.679, 316.85100, 93.39497, 1.3260732, 'GateDuelRing', 100),
    (503, 'StormwindGate', 12, 9, 0, -9129.000, 348.80000, 93.62000, 0.5799463, 'GateDuelRing', 100),
    (504, 'StormwindGate', 12, 9, 0, -9111.300, 314.20000, 93.40000, 1.3260732, 'GateDuelRing', 100),
    (505, 'StormwindGate', 12, 9, 0, -9123.800, 348.90000, 93.62000, 0.5799463, 'GateDuelRing', 100),
    (506, 'StormwindGate', 12, 9, 0, -9106.000, 314.40000, 93.40000, 1.3260732, 'GateDuelRing', 100),
    (601, 'OrgrimmarGate', 14, 9, 1, 1276.2615, -4400.968, 26.318865, 0.40867978, 'GateDuelRing', 100),
    (602, 'OrgrimmarGate', 14, 9, 1, 1283.0000, -4394.500, 26.350000, 0.40867978, 'GateDuelRing', 100),
    (603, 'OrgrimmarGate', 14, 9, 1, 1273.6000, -4403.200, 26.320000, 0.40867978, 'GateDuelRing', 100),
    (604, 'OrgrimmarGate', 14, 9, 1, 1280.5000, -4391.900, 26.350000, 0.40867978, 'GateDuelRing', 100),
    (605, 'OrgrimmarGate', 14, 9, 1, 1278.8000, -4403.000, 26.320000, 0.40867978, 'GateDuelRing', 100),
    (606, 'OrgrimmarGate', 14, 9, 1, 1285.6000, -4392.200, 26.350000, 0.40867978, 'GateDuelRing', 100)
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

INSERT INTO `city_bot_poi`
    (`id`, `location_key`, `zone_id`, `poi_type`, `map_id`, `pos_x`, `pos_y`, `pos_z`, `orientation`, `district`, `weight`)
VALUES
    (410, 'Goldshire', 12, 7, 0, -9488.0, 75.0, 56.0, 0.0, 'BlacksmithRoad', 100),
    (411, 'Goldshire', 12, 7, 0, -9449.0, 86.0, 56.0, 0.0, 'InnYard', 100),
    (412, 'Goldshire', 12, 7, 0, -9498.0, 38.0, 56.0, 0.0, 'WestRoad', 100),
    (413, 'Goldshire', 12, 7, 0, -9435.0, 60.0, 56.0, 0.0, 'EastRoad', 100),
    (414, 'Goldshire', 12, 7, 0, -9474.0, 42.0, 64.69, 4.65, 'UpstairsSpectator', 70),
    (415, 'Goldshire', 12, 7, 0, -9468.0, 31.0, 64.69, 4.65, 'UpstairsHall', 70),
    (111, 'Orgrimmar', 1637, 7, 1, 1565.0, -4360.0, 18.0, 0.0, 'ValleyMarket', 100),
    (112, 'Orgrimmar', 1637, 3, 1, 1615.0, -4438.0, 15.0, 0.0, 'ValleyInn', 100),
    (113, 'Orgrimmar', 1637, 7, 1, 1705.0, -4376.0, 31.0, 0.0, 'DragEntry', 100),
    (114, 'Orgrimmar', 1637, 7, 1, 1775.0, -4285.0, 8.0, 0.0, 'Drag', 100),
    (115, 'Orgrimmar', 1637, 8, 1, 1840.0, -4395.0, -15.0, 0.0, 'Cleft', 100),
    (116, 'Orgrimmar', 1637, 7, 1, 1505.0, -4415.0, 22.0, 0.0, 'GateWalk', 100),
    (117, 'Orgrimmar', 1637, 7, 1, 1470.0, -4315.0, 26.0, 0.0, 'GateWalk', 100),
    (118, 'Orgrimmar', 1637, 7, 1, 1690.0, -4270.0, 45.0, 0.0, 'Ridge', 80)
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
