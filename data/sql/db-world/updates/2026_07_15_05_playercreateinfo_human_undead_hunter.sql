-- Human Hunter + Undead Hunter: required for sObjectMgr::GetPlayerInfo() / InitDisplayIds().
-- Your realm already has player_class_stats (1,3) but was missing playercreateinfo rows.

DELETE FROM `playercreateinfo` WHERE `race` = 1 AND `class` = 3;
DELETE FROM `playercreateinfo` WHERE `race` = 5 AND `class` = 3;
INSERT INTO `playercreateinfo` (`race`, `class`, `map`, `zone`, `position_x`, `position_y`, `position_z`, `orientation`) VALUES
(1, 3, 0, 12, -8949.95, -132.493, 83.5312, 0),
(5, 3, 0, 85, 1676.71, 1678.31, 121.67, 2.70526);

DELETE FROM `playercreateinfo_action` WHERE `race` = 1 AND `class` = 3;
DELETE FROM `playercreateinfo_action` WHERE `race` = 5 AND `class` = 3;
INSERT INTO `playercreateinfo_action` (`race`, `class`, `button`, `action`, `type`) VALUES
(1, 3, 0, 6603, 0),
(1, 3, 1, 2973, 0),
(1, 3, 2, 75, 0),
(1, 3, 3, 59752, 0),
(1, 3, 4, 2481, 0),
(5, 3, 0, 6603, 0),
(5, 3, 1, 2973, 0),
(5, 3, 2, 75, 0),
(5, 3, 3, 7744, 0);
