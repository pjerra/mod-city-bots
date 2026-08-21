-- mod-city-bots V2 — citizen_roster (fixed cast)
DROP TABLE IF EXISTS `citizen_roster`;
CREATE TABLE `citizen_roster` (
    `guid` INT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL,
    `character_name` VARCHAR(12) NOT NULL,
    `race` TINYINT UNSIGNED NOT NULL,
    `class` TINYINT UNSIGNED NOT NULL,
    `gender` TINYINT UNSIGNED NOT NULL,
    `level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `home_key` VARCHAR(32) NOT NULL,
    `role` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `poi_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `login_priority` SMALLINT NOT NULL DEFAULT 100,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`guid`),
    UNIQUE KEY `idx_name` (`character_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DELETE FROM `citizen_roster`;

INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000001, 12001, 'Aldgate', 1, 1, 0, 80, 'StormwindGate', 3, 501, 0, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000002, 12001, 'Brenwick', 3, 2, 1, 80, 'StormwindGate', 3, 503, 1, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000003, 12001, 'Corven', 3, 3, 0, 80, 'StormwindGate', 3, 501, 2, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000004, 12001, 'Daxley', 7, 4, 1, 80, 'StormwindGate', 3, 504, 3, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000005, 12001, 'Eldon', 1, 5, 0, 80, 'StormwindGate', 3, 505, 4, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000006, 12001, 'Farris', 4, 11, 1, 80, 'StormwindGate', 3, 502, 5, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000007, 12001, 'Garrick', 1, 9, 0, 80, 'StormwindGate', 3, 501, 6, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000008, 12001, 'Hadrian', 11, 2, 1, 80, 'StormwindGate', 3, 503, 7, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000009, 12001, 'Iven', 11, 7, 0, 80, 'StormwindGate', 3, 506, 8, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000010, 12001, 'Jorick', 4, 11, 1, 80, 'StormwindGate', 3, 504, 9, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000011, 12002, 'Kelden', 7, 4, 0, 80, 'StormwindGate', 3, 505, 10, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000012, 12002, 'Lareth', 3, 1, 1, 80, 'StormwindGate', 3, 502, 11, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000013, 12002, 'Thrakka', 2, 1, 0, 80, 'OrgrimmarGate', 3, 601, 20, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000014, 12002, 'Mokren', 2, 7, 1, 80, 'OrgrimmarGate', 3, 602, 21, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000015, 12002, 'Zulvek', 2, 3, 0, 80, 'OrgrimmarGate', 3, 603, 22, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000016, 12002, 'Grashna', 2, 4, 1, 80, 'OrgrimmarGate', 3, 604, 23, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000017, 12002, 'Rokmar', 2, 9, 0, 80, 'OrgrimmarGate', 3, 605, 24, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000018, 12002, 'Durgoth', 8, 3, 1, 80, 'OrgrimmarGate', 3, 606, 25, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000019, 12002, 'Hazrak', 8, 7, 0, 80, 'OrgrimmarGate', 3, 601, 26, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000020, 12002, 'Korgath', 8, 8, 1, 80, 'OrgrimmarGate', 3, 602, 27, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000021, 12003, 'Mazrek', 5, 5, 0, 80, 'OrgrimmarGate', 3, 603, 28, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000022, 12003, 'Nazgul', 5, 9, 1, 80, 'OrgrimmarGate', 3, 604, 29, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000023, 12003, 'Orgrim', 10, 2, 0, 80, 'OrgrimmarGate', 3, 605, 30, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000024, 12003, 'Pazzik', 10, 4, 1, 80, 'OrgrimmarGate', 3, 606, 31, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000025, 12003, 'Liora', 1, 5, 1, 20, 'Goldshire', 1, 404, 40, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000026, 12003, 'Mira', 1, 8, 1, 22, 'Goldshire', 1, 405, 41, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000027, 12003, 'Denton', 1, 1, 0, 80, 'Goldshire', 4, 408, 42, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000028, 12003, 'Rylee', 1, 4, 0, 80, 'Goldshire', 4, 409, 43, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000029, 12003, 'Tomas', 1, 1, 0, 18, 'Goldshire', 2, 407, 44, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000030, 12003, 'Ella', 1, 5, 1, 19, 'Goldshire', 2, 403, 45, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000031, 12004, 'Finn', 1, 8, 0, 21, 'Goldshire', 2, 407, 46, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000032, 12004, 'Greta', 1, 3, 1, 20, 'Goldshire', 2, 408, 47, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000033, 12004, 'Elwyn', 1, 1, 0, 12, 'Goldshire', 0, 401, 32, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000034, 12004, 'Yorgrim', 3, 2, 1, 13, 'Goldshire', 0, 402, 33, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000035, 12004, 'Elarriel', 4, 4, 0, 14, 'Goldshire', 0, 403, 34, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000036, 12004, 'Fizfizz', 7, 9, 1, 15, 'Goldshire', 0, 404, 35, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000037, 12004, 'Aaliel', 11, 7, 0, 16, 'Goldshire', 0, 405, 36, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000038, 12004, 'Perrwyn', 1, 8, 1, 17, 'Goldshire', 0, 406, 37, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000039, 12004, 'Farrik', 3, 1, 0, 18, 'Goldshire', 0, 407, 38, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000040, 12004, 'Feythera', 4, 3, 1, 19, 'Goldshire', 0, 408, 39, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000041, 12005, 'Jixgear', 7, 1, 0, 20, 'Goldshire', 0, 409, 40, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000042, 12005, 'Naariel', 11, 5, 1, 21, 'Goldshire', 0, 410, 41, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000043, 12005, 'Branton', 1, 4, 0, 22, 'Goldshire', 0, 411, 42, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000044, 12005, 'Torrik', 3, 3, 1, 23, 'Goldshire', 0, 412, 43, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000045, 12005, 'Thalthera', 4, 1, 0, 24, 'Goldshire', 0, 413, 44, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000046, 12005, 'Zimgear', 7, 4, 1, 25, 'Goldshire', 0, 414, 45, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000047, 12005, 'Daaniel', 11, 3, 0, 26, 'Goldshire', 0, 415, 46, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000048, 12005, 'Garrley', 1, 2, 1, 27, 'Goldshire', 0, 401, 47, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000049, 12005, 'Bromdan', 3, 5, 0, 28, 'Goldshire', 0, 402, 48, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000050, 12005, 'Jorley', 1, 1, 0, 15, 'Stormwind', 0, 1, 49, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000051, 12006, 'Fardan', 3, 2, 1, 16, 'Stormwind', 0, 2, 50, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000052, 12006, 'Feywyn', 4, 4, 0, 17, 'Stormwind', 0, 3, 51, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000053, 12006, 'Jixlock', 7, 9, 1, 18, 'Stormwind', 0, 4, 52, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000054, 12006, 'Naaruun', 11, 7, 0, 19, 'Stormwind', 0, 5, 53, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000055, 12006, 'Wesley', 1, 8, 1, 20, 'Stormwind', 0, 6, 54, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000056, 12006, 'Tordan', 3, 1, 0, 21, 'Stormwind', 0, 7, 55, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000057, 12006, 'Thalwyn', 4, 3, 1, 22, 'Stormwind', 0, 8, 56, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000058, 12006, 'Zimlock', 7, 1, 0, 23, 'Stormwind', 0, 9, 57, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000059, 12006, 'Daanuun', 11, 5, 1, 24, 'Stormwind', 0, 10, 58, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000060, 12006, 'Dawton', 1, 4, 0, 25, 'Stormwind', 0, 11, 59, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000061, 12007, 'Brommar', 3, 3, 1, 26, 'Stormwind', 0, 12, 60, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000062, 12007, 'Beldris', 4, 1, 0, 27, 'Stormwind', 0, 13, 61, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000063, 12007, 'Fizzspark', 7, 4, 1, 28, 'Stormwind', 0, 14, 62, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000064, 12007, 'Kyrara', 11, 3, 0, 29, 'Stormwind', 0, 15, 63, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000065, 12007, 'Marton', 1, 2, 1, 30, 'Stormwind', 0, 1, 64, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000066, 12007, 'Kragmar', 3, 5, 0, 31, 'Stormwind', 0, 2, 65, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000067, 12007, 'Naradris', 4, 11, 1, 32, 'Stormwind', 0, 3, 66, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000068, 12007, 'Tinkspark', 7, 8, 0, 33, 'Stormwind', 0, 4, 67, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000069, 12007, 'Veyara', 11, 2, 1, 34, 'Stormwind', 0, 5, 68, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000070, 12007, 'Arton', 1, 9, 0, 35, 'Stormwind', 0, 6, 69, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000071, 12008, 'Barmar', 3, 4, 1, 36, 'Stormwind', 0, 7, 70, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000072, 12008, 'Saridris', 4, 11, 0, 37, 'Stormwind', 0, 8, 71, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000073, 12008, 'Bixwhiz', 7, 9, 1, 38, 'Stormwind', 0, 9, 72, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000074, 12008, 'Eliesh', 11, 1, 0, 39, 'Stormwind', 0, 10, 73, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000075, 12008, 'Elford', 1, 5, 1, 40, 'Stormwind', 0, 11, 74, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000076, 12008, 'Grimstag', 3, 3, 0, 41, 'Stormwind', 0, 12, 75, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000077, 12008, 'Lethnara', 4, 5, 1, 42, 'Stormwind', 0, 13, 76, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000078, 12008, 'Nibwhiz', 7, 1, 0, 43, 'Stormwind', 0, 14, 77, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000079, 12008, 'Orelesh', 11, 8, 1, 44, 'Stormwind', 0, 15, 78, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000080, 12008, 'Perrford', 1, 3, 0, 45, 'Stormwind', 0, 1, 79, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000081, 12009, 'Ulfstag', 3, 2, 1, 46, 'Stormwind', 0, 2, 80, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000082, 12009, 'Veynara', 4, 4, 0, 47, 'Stormwind', 0, 3, 81, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000083, 12009, 'Mekwhiz', 7, 4, 1, 48, 'Stormwind', 0, 4, 82, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000084, 12009, 'Lumiesh', 11, 7, 0, 49, 'Stormwind', 0, 5, 83, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000085, 12009, 'Alren', 1, 1, 1, 50, 'Stormwind', 0, 6, 84, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000086, 12009, 'Dunbar', 3, 1, 0, 51, 'Stormwind', 0, 7, 85, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000087, 12009, 'Delsong', 4, 3, 1, 52, 'Stormwind', 0, 8, 86, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000088, 12009, 'Gimwire', 7, 8, 0, 53, 'Stormwind', 0, 9, 87, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000089, 12009, 'Maraion', 11, 5, 1, 54, 'Stormwind', 0, 10, 88, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000090, 12009, 'Garrren', 1, 1, 0, 55, 'Stormwind', 0, 11, 89, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000091, 12010, 'Tharbar', 3, 3, 1, 56, 'Stormwind', 0, 12, 90, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000092, 12010, 'Sylsong', 4, 1, 0, 57, 'Stormwind', 0, 13, 91, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000093, 12010, 'Wixwire', 7, 8, 1, 58, 'Stormwind', 0, 14, 92, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000094, 12010, 'Yrelion', 11, 3, 0, 59, 'Stormwind', 0, 15, 93, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000095, 12010, 'Rowren', 1, 6, 1, 60, 'Stormwind', 0, 1, 94, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000096, 12010, 'Dornbar', 3, 5, 0, 61, 'Stormwind', 0, 2, 95, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000097, 12010, 'Arileaf', 4, 11, 1, 62, 'Stormwind', 0, 3, 96, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000098, 12010, 'Cogsnap', 7, 8, 0, 63, 'Stormwind', 0, 4, 97, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000099, 12010, 'Ishria', 11, 1, 1, 64, 'Stormwind', 0, 5, 98, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000100, 12010, 'Branwick', 1, 2, 0, 15, 'Stormwind', 0, 6, 99, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000101, 12011, 'Harren', 3, 4, 1, 16, 'Stormwind', 0, 7, 100, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000102, 12011, 'Maerleaf', 4, 1, 0, 17, 'Stormwind', 0, 8, 101, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000103, 12011, 'Pipsnap', 7, 4, 1, 18, 'Stormwind', 0, 9, 102, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000104, 12011, 'Samaria', 11, 1, 0, 19, 'Stormwind', 0, 10, 103, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000105, 12011, 'Halwick', 1, 9, 1, 20, 'Stormwind', 0, 11, 104, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000106, 12011, 'Yorren', 3, 3, 0, 21, 'Stormwind', 0, 12, 105, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000107, 12011, 'Elarleaf', 4, 5, 1, 22, 'Stormwind', 0, 13, 106, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000108, 12011, 'Fizsnap', 7, 8, 0, 23, 'Stormwind', 0, 14, 107, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000109, 12011, 'Aalira', 11, 8, 1, 24, 'Stormwind', 0, 15, 108, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000110, 12011, 'Tavwick', 1, 5, 0, 25, 'Stormwind', 0, 1, 109, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000111, 12012, 'Fardin', 3, 2, 1, 26, 'Stormwind', 0, 2, 110, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000112, 12012, 'Feydorn', 4, 4, 0, 27, 'Stormwind', 0, 3, 111, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000113, 12012, 'Jixbit', 7, 9, 1, 28, 'Stormwind', 0, 4, 112, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000114, 12012, 'Naarira', 11, 7, 0, 29, 'Stormwind', 0, 5, 113, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000115, 12012, 'Cedlan', 1, 3, 1, 30, 'Stormwind', 0, 6, 114, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000116, 12012, 'Tordin', 3, 1, 0, 31, 'Stormwind', 0, 7, 115, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000117, 12012, 'Thaldorn', 4, 3, 1, 32, 'Stormwind', 0, 8, 116, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000118, 12012, 'Zimbit', 7, 1, 0, 33, 'Stormwind', 0, 9, 117, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000119, 12012, 'Daanira', 11, 5, 1, 34, 'Stormwind', 0, 10, 118, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000120, 12012, 'Jorlan', 1, 1, 0, 35, 'Stormwind', 0, 11, 119, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000121, 12013, 'Bromgar', 3, 3, 1, 36, 'Stormwind', 0, 12, 120, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000122, 12013, 'Bellara', 4, 1, 0, 37, 'Stormwind', 0, 13, 121, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000123, 12013, 'Fizzbolt', 7, 4, 1, 38, 'Stormwind', 0, 14, 122, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000124, 12013, 'Kyraan', 11, 3, 0, 39, 'Stormwind', 0, 15, 123, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000125, 12013, 'Weslan', 1, 8, 1, 40, 'Stormwind', 0, 1, 124, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000126, 12013, 'Rokgoth', 2, 1, 0, 15, 'Orgrimmar', 0, 101, 125, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000127, 12013, 'Morwin', 5, 3, 1, 16, 'Orgrimmar', 0, 102, 126, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000128, 12013, 'Tahutotem', 6, 7, 0, 17, 'Orgrimmar', 0, 103, 127, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000129, 12013, 'Volra', 8, 5, 1, 18, 'Orgrimmar', 0, 104, 128, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000130, 12013, 'Zanaris', 10, 8, 0, 19, 'Orgrimmar', 0, 105, 129, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000131, 12014, 'Zuggoth', 2, 9, 1, 20, 'Orgrimmar', 0, 106, 130, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000132, 12014, 'Hestwin', 5, 1, 0, 21, 'Orgrimmar', 0, 107, 131, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000133, 12014, 'Hamucloud', 6, 7, 1, 22, 'Orgrimmar', 0, 108, 132, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000134, 12014, 'Bwemzul', 8, 4, 0, 23, 'Orgrimmar', 0, 109, 133, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000135, 12014, 'Caelther', 10, 5, 1, 24, 'Orgrimmar', 0, 110, 134, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000136, 12014, 'Mokmak', 2, 3, 0, 25, 'Orgrimmar', 0, 111, 135, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000137, 12014, 'Edgrave', 5, 9, 1, 26, 'Orgrimmar', 0, 112, 136, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000138, 12014, 'Tahucloud', 6, 7, 0, 27, 'Orgrimmar', 0, 113, 137, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000139, 12014, 'Senzul', 8, 3, 1, 28, 'Orgrimmar', 0, 114, 138, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000140, 12014, 'Valether', 10, 4, 0, 29, 'Orgrimmar', 0, 115, 139, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000141, 12015, 'Urokmak', 2, 7, 1, 30, 'Orgrimmar', 0, 101, 140, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000142, 12015, 'Velgrave', 5, 8, 0, 31, 'Orgrimmar', 0, 102, 141, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000143, 12015, 'Hamuwind', 6, 7, 1, 32, 'Orgrimmar', 0, 103, 142, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000144, 12015, 'Mekzul', 8, 1, 0, 33, 'Orgrimmar', 0, 104, 143, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000145, 12015, 'Aelrien', 10, 3, 1, 34, 'Orgrimmar', 0, 105, 144, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000146, 12015, 'Gorrok', 2, 4, 0, 35, 'Orgrimmar', 0, 106, 145, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000147, 12015, 'Caldmere', 5, 5, 1, 36, 'Orgrimmar', 0, 107, 146, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000148, 12015, 'Tahuwind', 6, 7, 0, 37, 'Orgrimmar', 0, 108, 147, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000149, 12015, 'Kaztik', 8, 8, 1, 38, 'Orgrimmar', 0, 109, 148, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000150, 12015, 'Selrien', 10, 2, 0, 39, 'Orgrimmar', 0, 110, 149, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000151, 12016, 'Skarnrok', 2, 3, 1, 40, 'Orgrimmar', 0, 111, 150, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000152, 12016, 'Nethmere', 5, 4, 0, 41, 'Orgrimmar', 0, 112, 151, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000153, 12016, 'Hamuplain', 6, 7, 1, 42, 'Orgrimmar', 0, 113, 152, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000154, 12016, 'Zantik', 8, 7, 0, 43, 'Orgrimmar', 0, 114, 153, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000155, 12016, 'Renrien', 10, 9, 1, 44, 'Orgrimmar', 0, 115, 154, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000156, 12016, 'Drogrok', 2, 1, 0, 45, 'Orgrimmar', 0, 101, 155, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000157, 12016, 'Alarwick', 5, 3, 1, 46, 'Orgrimmar', 0, 102, 156, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000158, 12016, 'Tahuplain', 6, 7, 0, 47, 'Orgrimmar', 0, 103, 157, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000159, 12016, 'Jazza', 8, 5, 1, 48, 'Orgrimmar', 0, 104, 158, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000160, 12016, 'Felolara', 10, 8, 0, 49, 'Orgrimmar', 0, 105, 159, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000161, 12017, 'Nazgar', 2, 9, 1, 50, 'Orgrimmar', 0, 106, 160, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000162, 12017, 'Fenwick', 5, 1, 0, 51, 'Orgrimmar', 0, 107, 161, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000163, 12017, 'Hamuhorn', 6, 7, 1, 52, 'Orgrimmar', 0, 108, 162, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000164, 12017, 'Tazza', 8, 4, 0, 53, 'Orgrimmar', 0, 109, 163, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000165, 12017, 'Vellara', 10, 5, 1, 54, 'Orgrimmar', 0, 110, 164, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000166, 12017, 'Varkgar', 2, 3, 0, 55, 'Orgrimmar', 0, 111, 165, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000167, 12017, 'Weswick', 5, 9, 1, 56, 'Orgrimmar', 0, 112, 166, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000168, 12017, 'Tahuhorn', 6, 1, 0, 57, 'Orgrimmar', 0, 113, 167, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000169, 12017, 'Akamon', 8, 3, 1, 58, 'Orgrimmar', 0, 114, 168, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000170, 12017, 'Beldrel', 10, 4, 0, 59, 'Orgrimmar', 0, 115, 169, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000171, 12018, 'Kazvek', 2, 6, 1, 60, 'Orgrimmar', 0, 101, 170, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000172, 12018, 'Drelton', 5, 6, 0, 61, 'Orgrimmar', 0, 102, 171, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000173, 12018, 'Hamumane', 6, 11, 1, 62, 'Orgrimmar', 0, 103, 172, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000174, 12018, 'Rokmon', 8, 8, 0, 63, 'Orgrimmar', 0, 104, 173, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000175, 12018, 'Talardrel', 10, 2, 1, 64, 'Orgrimmar', 0, 105, 174, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000176, 12018, 'Thrakvek', 2, 4, 0, 15, 'Orgrimmar', 0, 106, 175, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000177, 12018, 'Oston', 5, 5, 1, 16, 'Orgrimmar', 0, 107, 176, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000178, 12018, 'Tahumane', 6, 7, 0, 17, 'Orgrimmar', 0, 108, 177, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000179, 12018, 'Zulmon', 8, 8, 1, 18, 'Orgrimmar', 0, 109, 178, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000180, 12018, 'Saedrel', 10, 2, 0, 19, 'Orgrimmar', 0, 110, 179, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000181, 12019, 'Gargul', 2, 3, 1, 20, 'Orgrimmar', 0, 111, 180, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000182, 12019, 'Blyford', 5, 4, 0, 21, 'Orgrimmar', 0, 112, 181, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000183, 12019, 'Hamurun', 6, 7, 1, 22, 'Orgrimmar', 0, 113, 182, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000184, 12019, 'Jinrek', 8, 7, 0, 23, 'Orgrimmar', 0, 114, 183, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000185, 12019, 'Loraanor', 10, 9, 1, 24, 'Orgrimmar', 0, 115, 184, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000186, 12019, 'Rokgul', 2, 1, 0, 25, 'Orgrimmar', 0, 101, 185, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000187, 12019, 'Morford', 5, 3, 1, 26, 'Orgrimmar', 0, 102, 186, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000188, 12019, 'Tahurun', 6, 7, 0, 27, 'Orgrimmar', 0, 103, 187, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000189, 12019, 'Volrek', 8, 5, 1, 28, 'Orgrimmar', 0, 104, 188, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000190, 12019, 'Zananor', 10, 8, 0, 29, 'Orgrimmar', 0, 105, 189, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000191, 12020, 'Zuggul', 2, 9, 1, 30, 'Orgrimmar', 0, 106, 190, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000192, 12020, 'Hestford', 5, 1, 0, 31, 'Orgrimmar', 0, 107, 191, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000193, 12020, 'Hamuhoof', 6, 7, 1, 32, 'Orgrimmar', 0, 108, 192, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000194, 12020, 'Bwemji', 8, 4, 0, 33, 'Orgrimmar', 0, 109, 193, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000195, 12020, 'Caelriel', 10, 5, 1, 34, 'Orgrimmar', 0, 110, 194, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000196, 12020, 'Moknak', 2, 1, 0, 35, 'Orgrimmar', 0, 111, 195, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000197, 12020, 'Edwell', 5, 9, 1, 36, 'Orgrimmar', 0, 112, 196, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000198, 12020, 'Tahuhoof', 6, 7, 0, 37, 'Orgrimmar', 0, 113, 197, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000199, 12020, 'Senji', 8, 3, 1, 38, 'Orgrimmar', 0, 114, 198, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000200, 12020, 'Valeriel', 10, 4, 0, 39, 'Orgrimmar', 0, 115, 199, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000201, 12021, 'Elton', 1, 1, 0, 30, 'Dalaran', 0, 1201, 200, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000202, 12021, 'Zanriel', 10, 3, 1, 31, 'Dalaran', 0, 1202, 201, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000203, 12021, 'Daanion', 11, 3, 0, 32, 'Dalaran', 0, 1203, 202, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000204, 12021, 'Drognak', 2, 7, 1, 33, 'Dalaran', 0, 1204, 203, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000205, 12021, 'Alarrow', 5, 8, 0, 34, 'Dalaran', 0, 1205, 204, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000206, 12021, 'Perrton', 1, 8, 1, 35, 'Dalaran', 0, 1206, 205, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000207, 12021, 'Caelvyn', 10, 2, 0, 36, 'Dalaran', 0, 1207, 206, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000208, 12021, 'Kyrria', 11, 2, 1, 37, 'Dalaran', 0, 1208, 207, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000209, 12021, 'Nazmog', 2, 4, 0, 38, 'Dalaran', 0, 1209, 208, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000210, 12021, 'Fenrow', 5, 5, 1, 39, 'Dalaran', 0, 1210, 209, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000211, 12022, 'Alford', 1, 4, 0, 40, 'Dalaran', 0, 1211, 210, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000212, 12022, 'Valevyn', 10, 9, 1, 41, 'Dalaran', 0, 1212, 211, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000213, 12022, 'Veyria', 11, 1, 0, 42, 'Dalaran', 0, 1213, 212, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000214, 12022, 'Varkmog', 2, 3, 1, 43, 'Dalaran', 0, 1214, 213, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000215, 12022, 'Wesrow', 5, 4, 0, 44, 'Dalaran', 0, 1215, 214, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000216, 12022, 'Garrford', 1, 2, 1, 45, 'Dalaran', 0, 1201, 215, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000217, 12022, 'Aelion', 10, 8, 0, 46, 'Dalaran', 0, 1202, 216, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000218, 12022, 'Eliira', 11, 8, 1, 47, 'Dalaran', 0, 1203, 217, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000219, 12022, 'Kazash', 2, 1, 0, 48, 'Dalaran', 0, 1204, 218, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000220, 12022, 'Drelric', 5, 3, 1, 49, 'Dalaran', 0, 1205, 219, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000221, 12023, 'Rowford', 1, 9, 0, 50, 'Dalaran', 0, 1206, 220, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000222, 12023, 'Selion', 10, 5, 1, 51, 'Dalaran', 0, 1207, 221, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000223, 12023, 'Orelira', 11, 7, 0, 52, 'Dalaran', 0, 1208, 222, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000224, 12023, 'Thrakash', 2, 9, 1, 53, 'Dalaran', 0, 1209, 223, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000225, 12023, 'Velwin', 5, 1, 0, 54, 'Dalaran', 0, 1210, 224, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000226, 12023, 'Branren', 1, 2, 1, 55, 'Dalaran', 0, 1211, 225, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000227, 12023, 'Renion', 10, 8, 0, 56, 'Dalaran', 0, 1212, 226, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000228, 12023, 'Lumiira', 11, 8, 1, 57, 'Dalaran', 0, 1213, 227, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000229, 12023, 'Gargoth', 2, 1, 0, 58, 'Dalaran', 0, 1214, 228, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000230, 12023, 'Blywin', 5, 3, 1, 59, 'Dalaran', 0, 1215, 229, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000231, 12024, 'Perrley', 1, 8, 0, 60, 'Dalaran', 0, 1201, 230, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000232, 12024, 'Hardan', 10, 5, 1, 61, 'Dalaran', 0, 1202, 231, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000233, 12024, 'Maerwyn', 11, 6, 0, 62, 'Dalaran', 0, 1203, 232, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000234, 12024, 'Piplock', 7, 4, 1, 63, 'Dalaran', 0, 1204, 233, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000235, 12024, 'Samauun', 5, 9, 0, 64, 'Dalaran', 0, 1205, 234, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000236, 12024, 'Alton', 1, 4, 1, 65, 'Dalaran', 0, 1206, 235, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000237, 12024, 'Yordan', 10, 3, 0, 66, 'Dalaran', 0, 1207, 236, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000238, 12024, 'Elarwyn', 11, 3, 1, 67, 'Dalaran', 0, 1208, 237, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000239, 12024, 'Fizlock', 2, 6, 0, 68, 'Dalaran', 0, 1209, 238, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000240, 12024, 'Aalara', 5, 6, 1, 69, 'Dalaran', 0, 1210, 239, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000241, 12025, 'Cedwick', 1, 1, 0, 30, 'Shattrath', 0, 1101, 240, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000242, 12025, 'Belther', 10, 3, 1, 31, 'Shattrath', 0, 1102, 241, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000243, 12025, 'Ishdor', 11, 3, 0, 32, 'Shattrath', 0, 1103, 242, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000244, 12025, 'Nazrok', 2, 7, 1, 33, 'Shattrath', 0, 1104, 243, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000245, 12025, 'Fenmere', 5, 8, 0, 34, 'Shattrath', 0, 1105, 244, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000246, 12025, 'Marlan', 1, 8, 1, 35, 'Shattrath', 0, 1106, 245, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000247, 12025, 'Talarther', 10, 2, 0, 36, 'Shattrath', 0, 1107, 246, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000248, 12025, 'Samador', 11, 2, 1, 37, 'Shattrath', 0, 1108, 247, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000249, 12025, 'Varkrok', 2, 4, 0, 38, 'Shattrath', 0, 1109, 248, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000250, 12025, 'Wesmere', 5, 5, 1, 39, 'Shattrath', 0, 1110, 249, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000251, 12026, 'Arlan', 1, 4, 0, 40, 'Shattrath', 0, 1111, 250, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000252, 12026, 'Saether', 10, 9, 1, 41, 'Shattrath', 0, 1112, 251, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000253, 12026, 'Eliuun', 11, 1, 0, 42, 'Shattrath', 0, 1113, 252, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000254, 12026, 'Kazgar', 2, 3, 1, 43, 'Shattrath', 0, 1114, 253, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000255, 12026, 'Drelwick', 5, 4, 0, 44, 'Shattrath', 0, 1115, 254, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000256, 12026, 'Dawlan', 1, 2, 1, 45, 'Shattrath', 0, 1101, 255, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000257, 12026, 'Lorarien', 10, 8, 0, 46, 'Shattrath', 0, 1102, 256, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000258, 12026, 'Oreluun', 11, 8, 1, 47, 'Shattrath', 0, 1103, 257, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000259, 12026, 'Thrakgar', 2, 1, 0, 48, 'Shattrath', 0, 1104, 258, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000260, 12026, 'Oswick', 5, 3, 1, 49, 'Shattrath', 0, 1105, 259, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000261, 12027, 'Tavford', 1, 9, 0, 50, 'Shattrath', 0, 1106, 260, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000262, 12027, 'Renanor', 10, 5, 1, 51, 'Shattrath', 0, 1107, 261, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000263, 12027, 'Aalion', 11, 7, 0, 52, 'Shattrath', 0, 1108, 262, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000264, 12027, 'Gormog', 2, 9, 1, 53, 'Shattrath', 0, 1109, 263, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000265, 12027, 'Caldrow', 5, 1, 0, 54, 'Shattrath', 0, 1110, 264, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000266, 12027, 'Cedren', 1, 2, 1, 55, 'Shattrath', 0, 1111, 265, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000267, 12027, 'Feloriel', 10, 8, 0, 56, 'Shattrath', 0, 1112, 266, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000268, 12027, 'Naarria', 11, 8, 1, 57, 'Shattrath', 0, 1113, 267, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000269, 12027, 'Skarnmog', 2, 1, 0, 58, 'Shattrath', 0, 1114, 268, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000270, 12027, 'Nethrow', 5, 3, 1, 59, 'Shattrath', 0, 1115, 269, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000271, 12028, 'Elmond', 1, 1, 0, 15, 'Ironforge', 0, 201, 270, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000272, 12028, 'Ulfmar', 3, 2, 1, 16, 'Ironforge', 0, 202, 271, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000273, 12028, 'Veydris', 4, 4, 0, 17, 'Ironforge', 0, 203, 272, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000274, 12028, 'Mekspark', 7, 9, 1, 18, 'Ironforge', 0, 204, 273, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000275, 12028, 'Lumiara', 11, 7, 0, 19, 'Ironforge', 0, 205, 274, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000276, 12028, 'Perrmond', 1, 8, 1, 20, 'Ironforge', 0, 206, 275, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000277, 12028, 'Dunstag', 3, 1, 0, 21, 'Ironforge', 0, 207, 276, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000278, 12028, 'Delnara', 4, 3, 1, 22, 'Ironforge', 0, 208, 277, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000279, 12028, 'Gimwhiz', 7, 1, 0, 23, 'Ironforge', 0, 209, 278, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000280, 12028, 'Maraesh', 11, 5, 1, 24, 'Ironforge', 0, 210, 279, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000281, 12029, 'Alric', 1, 4, 0, 25, 'Ironforge', 0, 211, 280, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000282, 12029, 'Tharstag', 3, 4, 1, 26, 'Ironforge', 0, 212, 281, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000283, 12029, 'Sylnara', 4, 1, 0, 27, 'Ironforge', 0, 213, 282, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000284, 12029, 'Wixwhiz', 7, 4, 1, 28, 'Ironforge', 0, 214, 283, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000285, 12029, 'Yrelesh', 11, 3, 0, 29, 'Ironforge', 0, 215, 284, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000286, 12029, 'Garrric', 1, 2, 1, 30, 'Ironforge', 0, 201, 285, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000287, 12029, 'Dornstag', 3, 5, 0, 31, 'Ironforge', 0, 202, 286, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000288, 12029, 'Arinara', 4, 11, 1, 32, 'Ironforge', 0, 203, 287, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000289, 12029, 'Cogwire', 7, 8, 0, 33, 'Ironforge', 0, 204, 288, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000290, 12029, 'Ishion', 11, 2, 1, 34, 'Ironforge', 0, 205, 289, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000291, 12030, 'Rowric', 1, 9, 0, 35, 'Ironforge', 0, 206, 290, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000292, 12030, 'Harbar', 3, 4, 1, 36, 'Ironforge', 0, 207, 291, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000293, 12030, 'Maersong', 4, 1, 0, 37, 'Ironforge', 0, 208, 292, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000294, 12030, 'Pipwire', 7, 9, 1, 38, 'Ironforge', 0, 209, 293, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000295, 12030, 'Samaion', 11, 1, 0, 39, 'Ironforge', 0, 210, 294, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000296, 12030, 'Thrakgul', 2, 1, 0, 15, 'Undercity', 0, 301, 295, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000297, 12030, 'Osford', 5, 3, 1, 16, 'Undercity', 0, 302, 296, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000298, 12030, 'Yetuwind', 6, 7, 0, 17, 'Undercity', 0, 303, 297, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000299, 12030, 'Zulrek', 8, 5, 1, 18, 'Undercity', 0, 304, 298, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000300, 12030, 'Saeanor', 10, 8, 0, 19, 'Undercity', 0, 305, 299, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000301, 12031, 'Garnak', 2, 9, 1, 20, 'Undercity', 0, 306, 300, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000302, 12031, 'Blywell', 5, 1, 0, 21, 'Undercity', 0, 307, 301, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000303, 12031, 'Kodoplain', 6, 7, 1, 22, 'Undercity', 0, 308, 302, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000304, 12031, 'Jinji', 8, 4, 0, 23, 'Undercity', 0, 309, 303, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000305, 12031, 'Lorariel', 10, 5, 1, 24, 'Undercity', 0, 310, 304, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000306, 12031, 'Roknak', 2, 3, 0, 25, 'Undercity', 0, 311, 305, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000307, 12031, 'Morwell', 5, 9, 1, 26, 'Undercity', 0, 312, 306, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000308, 12031, 'Yetuplain', 6, 7, 0, 27, 'Undercity', 0, 313, 307, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000309, 12031, 'Volji', 8, 3, 1, 28, 'Undercity', 0, 314, 308, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000310, 12031, 'Renvyn', 10, 4, 0, 29, 'Undercity', 0, 315, 309, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000311, 12032, 'Zugnak', 2, 7, 1, 30, 'Undercity', 0, 301, 310, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000312, 12032, 'Hestwell', 5, 8, 0, 31, 'Undercity', 0, 302, 311, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000313, 12032, 'Kodohorn', 6, 7, 1, 32, 'Undercity', 0, 303, 312, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000314, 12032, 'Bwemdak', 8, 1, 0, 33, 'Undercity', 0, 304, 313, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000315, 12032, 'Feloion', 10, 3, 1, 34, 'Undercity', 0, 305, 314, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000316, 12032, 'Mokmog', 2, 4, 0, 35, 'Undercity', 0, 306, 315, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000317, 12032, 'Edrow', 5, 5, 1, 36, 'Undercity', 0, 307, 316, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000318, 12032, 'Yetuhorn', 6, 7, 0, 37, 'Undercity', 0, 308, 317, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000319, 12032, 'Sendak', 8, 8, 1, 38, 'Undercity', 0, 309, 318, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000320, 12032, 'Velion', 10, 2, 0, 39, 'Undercity', 0, 310, 319, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000321, 12033, 'Weswyn', 1, 1, 0, 15, 'Darnassus', 0, 701, 320, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000322, 12033, 'Bardin', 3, 2, 1, 16, 'Darnassus', 0, 702, 321, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000323, 12033, 'Saridorn', 4, 4, 0, 17, 'Darnassus', 0, 703, 322, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000324, 12033, 'Bixbit', 7, 9, 1, 18, 'Darnassus', 0, 704, 323, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000325, 12033, 'Eliaan', 11, 7, 0, 19, 'Darnassus', 0, 705, 324, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000326, 12033, 'Dawley', 1, 8, 1, 20, 'Darnassus', 0, 706, 325, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000327, 12033, 'Hargrim', 3, 1, 0, 21, 'Darnassus', 0, 707, 326, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000328, 12033, 'Maerriel', 4, 3, 1, 22, 'Darnassus', 0, 708, 327, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000329, 12033, 'Pipfizz', 7, 1, 0, 23, 'Darnassus', 0, 709, 328, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000330, 12033, 'Veyiel', 11, 5, 1, 24, 'Darnassus', 0, 710, 329, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000331, 12034, 'Marley', 1, 4, 0, 25, 'Darnassus', 0, 711, 330, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000332, 12034, 'Barrik', 3, 4, 1, 26, 'Darnassus', 0, 712, 331, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000333, 12034, 'Sarithera', 4, 1, 0, 27, 'Darnassus', 0, 713, 332, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000334, 12034, 'Bixgear', 7, 4, 1, 28, 'Darnassus', 0, 714, 333, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000335, 12034, 'Aaldor', 11, 3, 0, 29, 'Darnassus', 0, 715, 334, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000336, 12034, 'Arley', 1, 2, 1, 30, 'Darnassus', 0, 701, 335, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000337, 12034, 'Dungrim', 3, 5, 0, 31, 'Darnassus', 0, 702, 336, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000338, 12034, 'Delriel', 4, 11, 1, 32, 'Darnassus', 0, 703, 337, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000339, 12034, 'Gimfizz', 7, 8, 0, 33, 'Darnassus', 0, 704, 338, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000340, 12034, 'Marador', 11, 2, 1, 34, 'Darnassus', 0, 705, 339, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000341, 12035, 'Tavden', 1, 1, 0, 15, 'Exodar', 0, 801, 340, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000342, 12035, 'Kraggar', 3, 2, 1, 16, 'Exodar', 0, 802, 341, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000343, 12035, 'Naralara', 4, 4, 0, 17, 'Exodar', 0, 803, 342, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000344, 12035, 'Tinkbolt', 7, 9, 1, 18, 'Exodar', 0, 804, 343, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000345, 12035, 'Veyaan', 11, 7, 0, 19, 'Exodar', 0, 805, 344, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000346, 12035, 'Almond', 1, 8, 1, 20, 'Exodar', 0, 806, 345, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000347, 12035, 'Bargar', 3, 1, 0, 21, 'Exodar', 0, 807, 346, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000348, 12035, 'Sarilara', 4, 3, 1, 22, 'Exodar', 0, 808, 347, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000349, 12035, 'Bixfizz', 7, 1, 0, 23, 'Exodar', 0, 809, 348, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000350, 12035, 'Elidor', 11, 5, 1, 24, 'Exodar', 0, 810, 349, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000351, 12036, 'Dawwyn', 1, 4, 0, 25, 'Exodar', 0, 811, 350, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000352, 12036, 'Harmar', 3, 1, 1, 26, 'Exodar', 0, 812, 351, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000353, 12036, 'Maerdris', 4, 1, 0, 27, 'Exodar', 0, 813, 352, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000354, 12036, 'Pipspark', 7, 4, 1, 28, 'Exodar', 0, 814, 353, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000355, 12036, 'Samaara', 11, 3, 0, 29, 'Exodar', 0, 815, 354, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000356, 12036, 'Marwyn', 1, 2, 1, 30, 'Exodar', 0, 801, 355, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000357, 12036, 'Yormar', 3, 5, 0, 31, 'Exodar', 0, 802, 356, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000358, 12036, 'Elardris', 4, 11, 1, 32, 'Exodar', 0, 803, 357, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000359, 12036, 'Fizspark', 7, 8, 0, 33, 'Exodar', 0, 804, 358, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000360, 12036, 'Aalesh', 11, 2, 1, 34, 'Exodar', 0, 805, 359, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000361, 12037, 'Garrok', 2, 1, 0, 15, 'ThunderBluff', 0, 901, 360, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000362, 12037, 'Blymere', 5, 3, 1, 16, 'ThunderBluff', 0, 902, 361, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000363, 12037, 'Hamutotem', 6, 7, 0, 17, 'ThunderBluff', 0, 903, 362, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000364, 12037, 'Jintik', 8, 5, 1, 18, 'ThunderBluff', 0, 904, 363, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000365, 12037, 'Sellara', 10, 8, 0, 19, 'ThunderBluff', 0, 905, 364, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000366, 12037, 'Rokrok', 2, 9, 1, 20, 'ThunderBluff', 0, 906, 365, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000367, 12037, 'Mormere', 5, 1, 0, 21, 'ThunderBluff', 0, 907, 366, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000368, 12037, 'Yetucloud', 6, 7, 1, 22, 'ThunderBluff', 0, 908, 367, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000369, 12037, 'Voltik', 8, 4, 0, 23, 'ThunderBluff', 0, 909, 368, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000370, 12037, 'Zanrien', 10, 5, 1, 24, 'ThunderBluff', 0, 910, 369, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000371, 12038, 'Zugrok', 2, 3, 0, 25, 'ThunderBluff', 0, 911, 370, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000372, 12038, 'Hestmere', 5, 9, 1, 26, 'ThunderBluff', 0, 912, 371, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000373, 12038, 'Kodowind', 6, 7, 0, 27, 'ThunderBluff', 0, 913, 372, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000374, 12038, 'Bwemza', 8, 3, 1, 28, 'ThunderBluff', 0, 914, 373, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000375, 12038, 'Caellara', 10, 4, 0, 29, 'ThunderBluff', 0, 915, 374, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000376, 12038, 'Mokgar', 2, 7, 1, 30, 'ThunderBluff', 0, 901, 375, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000377, 12038, 'Edwick', 5, 8, 0, 31, 'ThunderBluff', 0, 902, 376, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000378, 12038, 'Banoplain', 6, 7, 1, 32, 'ThunderBluff', 0, 903, 377, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000379, 12038, 'Senza', 8, 1, 0, 33, 'ThunderBluff', 0, 904, 378, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000380, 12038, 'Valelara', 10, 3, 1, 34, 'ThunderBluff', 0, 905, 379, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000381, 12039, 'Urokgar', 2, 1, 0, 15, 'Silvermoon', 0, 1001, 380, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000382, 12039, 'Velwick', 5, 3, 1, 16, 'Silvermoon', 0, 1002, 381, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000383, 12039, 'Mahhorn', 6, 7, 0, 17, 'Silvermoon', 0, 1003, 382, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000384, 12039, 'Mekza', 8, 5, 1, 18, 'Silvermoon', 0, 1004, 383, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000385, 12039, 'Aeldrel', 10, 8, 0, 19, 'Silvermoon', 0, 1005, 384, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000386, 12039, 'Gorvek', 2, 9, 1, 20, 'Silvermoon', 0, 1006, 385, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000387, 12039, 'Caldton', 5, 1, 0, 21, 'Silvermoon', 0, 1007, 386, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000388, 12039, 'Banohorn', 6, 7, 1, 22, 'Silvermoon', 0, 1008, 387, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000389, 12039, 'Kazmon', 8, 4, 0, 23, 'Silvermoon', 0, 1009, 388, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000390, 12039, 'Seldrel', 10, 5, 1, 24, 'Silvermoon', 0, 1010, 389, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000391, 12040, 'Skarnvek', 2, 3, 0, 25, 'Silvermoon', 0, 1011, 390, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000392, 12040, 'Nethton', 5, 9, 1, 26, 'Silvermoon', 0, 1012, 391, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000393, 12040, 'Mahmane', 6, 7, 0, 27, 'Silvermoon', 0, 1013, 392, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000394, 12040, 'Zanmon', 8, 3, 1, 28, 'Silvermoon', 0, 1014, 393, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000395, 12040, 'Rendrel', 10, 4, 0, 29, 'Silvermoon', 0, 1015, 394, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000396, 12040, 'Drogvek', 2, 7, 1, 30, 'Silvermoon', 0, 1001, 395, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000397, 12040, 'Alarford', 5, 8, 0, 31, 'Silvermoon', 0, 1002, 396, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000398, 12040, 'Banomane', 6, 7, 1, 32, 'Silvermoon', 0, 1003, 397, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000399, 12040, 'Jazrek', 8, 1, 0, 33, 'Silvermoon', 0, 1004, 398, 1);
INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, `class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, `enabled`) VALUES (9000400, 12040, 'Feloanor', 10, 3, 1, 34, 'Silvermoon', 0, 1005, 399, 1);
-- Mark stage cast accounts as citizen type (3)
-- Each permanent city bot needs its own auth account so all 400 can be online at once.
UPDATE `citizen_roster`
   SET `account_id` = 12001 + (`guid` - 9000001)
 WHERE `guid` BETWEEN 9000001 AND 9000400;

-- Keep the fixed stage cast out of normal random-playerbot maintenance.
-- The module logs these accounts in directly after MinRandomBots/MaxRandomBots completes.
DELETE FROM `playerbots_account_type`
WHERE `account_id` BETWEEN 12001 AND 12400;

INSERT INTO `playerbots_account_type` (`account_id`, `account_type`, `assignment_date`)
SELECT `account_id`, 3, NOW()
  FROM `citizen_roster`
 WHERE `guid` BETWEEN 9000001 AND 9000400
   AND `enabled` = 1
   AND `account_id` BETWEEN 12001 AND 12400
 GROUP BY `account_id`;

DELETE FROM `playerbots_random_bots`
WHERE `bot` BETWEEN 9000001 AND 9000400
  AND `event` IN ('add', 'logout', 'citizen', 'citizen_burst', 'randomize', 'teleport', 'update');

-- Final POI spread for clean installs.
UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 6)
       WHEN 0 THEN 501
       WHEN 1 THEN 502
       WHEN 2 THEN 503
       WHEN 3 THEN 504
       WHEN 4 THEN 505
       ELSE 506
   END
 WHERE `home_key` = 'StormwindGate'
   AND `role` = 3;

UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 6)
       WHEN 0 THEN 601
       WHEN 1 THEN 602
       WHEN 2 THEN 603
       WHEN 3 THEN 604
       WHEN 4 THEN 605
       ELSE 606
   END
 WHERE `home_key` = 'OrgrimmarGate'
   AND `role` = 3;

UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 10)
       WHEN 0 THEN 401
       WHEN 1 THEN 402
       WHEN 2 THEN 403
       WHEN 3 THEN 407
       WHEN 4 THEN 410
       WHEN 5 THEN 411
       WHEN 6 THEN 412
       WHEN 7 THEN 413
       WHEN 8 THEN 414
       ELSE 415
   END
 WHERE `home_key` = 'Goldshire'
   AND `role` IN (0, 2);

UPDATE `citizen_roster`
   SET `poi_id` = CASE MOD(`guid`, 16)
       WHEN 0 THEN 101
       WHEN 1 THEN 102
       WHEN 2 THEN 103
       WHEN 3 THEN 104
       WHEN 4 THEN 105
       WHEN 5 THEN 106
       WHEN 6 THEN 109
       WHEN 7 THEN 110
       WHEN 8 THEN 111
       WHEN 9 THEN 112
       WHEN 10 THEN 113
       WHEN 11 THEN 114
       WHEN 12 THEN 115
       WHEN 13 THEN 116
       WHEN 14 THEN 117
       ELSE 118
   END
 WHERE `home_key` = 'Orgrimmar'
   AND `role` = 0;

-- Dedicated fishers: two at Goldshire pond, Stormwind harbor, Orgrimmar dock, and Ironforge Forlorn Cavern.
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 416 WHERE `guid` = 9000048;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 417 WHERE `guid` = 9000049;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 7   WHERE `guid` = 9000050;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 11  WHERE `guid` = 9000051;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 107 WHERE `guid` = 9000126;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 119 WHERE `guid` = 9000127;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 205 WHERE `guid` = 9000271;
UPDATE `citizen_roster` SET `role` = 5, `poi_id` = 206 WHERE `guid` = 9000272;

-- Final identity shape: one fixed city-bot account per fixed city-bot character.
UPDATE `citizen_roster`
   SET `account_id` = 12001 + (`guid` - 9000001)
 WHERE `guid` BETWEEN 9000001 AND 9000400;
