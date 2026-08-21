-- Separate Goldshire's two dedicated fishing POIs so fishers do not stack.
UPDATE `city_bot_poi`
SET `pos_x` = -9432.982,
    `pos_y` = 147.8207,
    `pos_z` = 56.400475,
    `orientation` = 1.7737789
WHERE `id` = 416
  AND `location_key` = 'Goldshire';

UPDATE `city_bot_poi`
SET `pos_x` = -9449.83,
    `pos_y` = 144.70833,
    `pos_z` = 56.253647,
    `orientation` = 1.1336792
WHERE `id` = 417
  AND `location_key` = 'Goldshire';
