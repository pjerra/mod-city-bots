-- mod-city-bots diagnostic only. Safe to run; no writes.
-- Run against acore_playerbots. This joins acore_characters and acore_world.

SELECT 'identity_mismatch_roster_vs_characters' AS check_name,
       COUNT(*) AS bad_rows
FROM citizen_roster r
JOIN acore_characters.characters c ON c.guid = r.guid
WHERE r.enabled = 1
  AND (c.account <> r.account_id
       OR c.name <> r.character_name
       OR c.race <> r.race
       OR c.class <> r.class
       OR c.gender <> r.gender
       OR c.level <> r.level);

SELECT 'dueler_counts_by_home' AS check_name,
       r.home_key,
       SUM(r.role = 3) AS gate_duelers,
       SUM(r.role = 4) AS goldshire_duelers,
       COUNT(*) AS total_dedicated_duelers
FROM citizen_roster r
WHERE r.enabled = 1
  AND r.role IN (3, 4)
GROUP BY r.home_key
ORDER BY r.home_key;

SELECT 'duelers_with_bad_character_state' AS check_name,
       r.guid, r.character_name, r.home_key, r.role,
       r.level AS roster_level,
       c.level AS character_level,
       c.online, c.at_login,
       c.map, c.position_x, c.position_y, c.position_z
FROM citizen_roster r
JOIN acore_characters.characters c ON c.guid = r.guid
WHERE r.enabled = 1
  AND r.role IN (3, 4)
  AND (c.level <> 80 OR c.at_login <> 0 OR c.online <> 0)
ORDER BY r.home_key, r.login_priority, r.guid;

SELECT 'duelers_with_missing_or_wrong_poi' AS check_name,
       r.guid, r.character_name, r.home_key, r.role, r.poi_id,
       p.id AS found_poi_id, p.location_key, p.poi_type, p.map_id,
       p.pos_x, p.pos_y, p.pos_z
FROM citizen_roster r
LEFT JOIN acore_world.city_bot_poi p ON p.id = r.poi_id
WHERE r.enabled = 1
  AND r.role IN (3, 4)
  AND (p.id IS NULL
       OR p.location_key <> r.home_key
       OR p.poi_type <> 9)
ORDER BY r.home_key, r.login_priority, r.guid;

SELECT 'duelers_far_from_assigned_poi' AS check_name,
       r.guid, r.character_name, r.home_key, r.role, r.poi_id,
       c.map AS char_map, p.map_id AS poi_map,
       ROUND(c.position_x, 3) AS char_x,
       ROUND(c.position_y, 3) AS char_y,
       ROUND(c.position_z, 3) AS char_z,
       ROUND(p.pos_x, 3) AS poi_x,
       ROUND(p.pos_y, 3) AS poi_y,
       ROUND(p.pos_z, 3) AS poi_z,
       ROUND(SQRT(POW(c.position_x - p.pos_x, 2) + POW(c.position_y - p.pos_y, 2) + POW(c.position_z - p.pos_z, 2)), 2) AS yards_from_poi
FROM citizen_roster r
JOIN acore_characters.characters c ON c.guid = r.guid
JOIN acore_world.city_bot_poi p ON p.id = r.poi_id
WHERE r.enabled = 1
  AND r.role IN (3, 4)
  AND (c.map <> p.map_id
       OR SQRT(POW(c.position_x - p.pos_x, 2) + POW(c.position_y - p.pos_y, 2) + POW(c.position_z - p.pos_z, 2)) > 45)
ORDER BY yards_from_poi DESC, r.home_key, r.guid;

SELECT 'dueler_poi_stack_counts' AS check_name,
       r.home_key, r.poi_id,
       ROUND(p.pos_x, 3) AS pos_x,
       ROUND(p.pos_y, 3) AS pos_y,
       ROUND(p.pos_z, 3) AS pos_z,
       COUNT(*) AS duelers_on_poi,
       GROUP_CONCAT(r.character_name ORDER BY r.login_priority SEPARATOR ', ') AS bots
FROM citizen_roster r
JOIN acore_world.city_bot_poi p ON p.id = r.poi_id
WHERE r.enabled = 1
  AND r.role IN (3, 4)
GROUP BY r.home_key, r.poi_id, p.pos_x, p.pos_y, p.pos_z
HAVING COUNT(*) > 2
ORDER BY duelers_on_poi DESC, r.home_key, r.poi_id;

SELECT 'gate_dueler_roster' AS check_name,
       r.guid, r.character_name, r.home_key, r.role, r.poi_id,
       r.account_id AS roster_account,
       c.account AS character_account,
       c.name AS character_name,
       c.level, c.online, c.at_login,
       p.map_id, p.pos_x, p.pos_y, p.pos_z, p.orientation
FROM citizen_roster r
JOIN acore_characters.characters c ON c.guid = r.guid
LEFT JOIN acore_world.city_bot_poi p ON p.id = r.poi_id
WHERE r.enabled = 1
  AND r.role = 3
ORDER BY r.home_key, r.login_priority, r.guid;
