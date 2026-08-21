-- Ambient city NPC behavior (optional flavor).
-- smart_scripts.id is smallint (max 32767); use low ids per creature entry.
-- Stormwind guards: occasional emote and say while idle.

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (68, 1976) AND `source_type` = 0 AND `id` BETWEEN 1 AND 4;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(68, 0, 1, 0, 1, 0, 100, 1, 30000, 90000, 120000, 240000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Guard - random talk timer'),
(68, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 25, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Guard - emote onehandedtalk'),
(68, 0, 3, 0, 61, 0, 50, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Guard - say line'),
(68, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Guard - evade reset'),
(1976, 0, 1, 0, 1, 0, 100, 1, 45000, 120000, 150000, 300000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Patroller - random talk timer'),
(1976, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Patroller - emote bow'),
(1976, 0, 3, 0, 61, 0, 40, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Patroller - say line'),
(1976, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Stormwind Patroller - evade reset');

-- Orgrimmar guards (entry 3296 Orgrimmar Grunt)
DELETE FROM `smart_scripts` WHERE `entryorguid` = 3296 AND `source_type` = 0 AND `id` BETWEEN 1 AND 4;

INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_phase_mask`, `event_chance`, `event_flags`, `event_param1`, `event_param2`, `event_param3`, `event_param4`, `event_param5`, `action_type`, `action_param1`, `action_param2`, `action_param3`, `action_param4`, `action_param5`, `action_param6`, `target_type`, `target_param1`, `target_param2`, `target_param3`, `target_param4`, `target_x`, `target_y`, `target_z`, `target_o`, `comment`) VALUES
(3296, 0, 1, 0, 1, 0, 100, 1, 30000, 90000, 120000, 240000, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Orgrimmar Grunt - random talk timer'),
(3296, 0, 2, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 1, 25, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Orgrimmar Grunt - emote talk'),
(3296, 0, 3, 0, 61, 0, 50, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Orgrimmar Grunt - say line'),
(3296, 0, 4, 0, 61, 0, 100, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 'mod-city-bots: Orgrimmar Grunt - evade reset');
