-- mod-city-bots — remove achievements earned by the stage cast (GUIDs 9000001-9000400).
--
-- Repair for servers that ran the module before CitizenBots.BlockAchievements
-- existed. Citizen bots run on dedicated accounts, so mod-playerbots'
-- realm-first guard (RandomPlayerbotMgr::IsRandomBot) never covered them and
-- they could take realm firsts from real players.
--
-- Self-contained on purpose: touches acore_characters only, no join against
-- acore_playerbots.citizen_roster. Idempotent; safe to re-run.
--
-- Realm firsts become claimable again only after a worldserver restart:
-- AchievementGlobalMgr::LoadCompletedAchievements() builds the "already taken"
-- set from character_achievement once, at startup.
--
-- Run it while worldserver is stopped. A logged-in bot will not rewrite deleted
-- rows on its own (AchievementMgr::SaveToDB only writes entries whose `changed`
-- flag is set, and loading clears it), but anything a citizen completes during
-- that session still would -- and the realm-first set is only rebuilt at startup
-- either way.

-- Achievement-reward titles the bots were granted are left alone; they only
-- show on the bot's own character.

DELETE FROM `character_achievement`
 WHERE `guid` BETWEEN 9000001 AND 9000400;

DELETE FROM `character_achievement_progress`
 WHERE `guid` BETWEEN 9000001 AND 9000400;
