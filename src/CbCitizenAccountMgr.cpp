#include "CbCitizenAccountMgr.h"

#include "Ai/City/CityBots/CitizenInfo.h"
#include "CitizenRosterRegistry.h"

#include "AccountMgr.h"
#include "CbLog.h"
#include "CbShippedSql.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "QueryResult.h"

#include <string>

namespace
{
    bool _validated = false;
}

namespace CbCitizenAccountMgr
{
    bool IsStageCastAccount(uint32 accountId)
    {
        return accountId >= STAGE_CAST_ACCOUNT_START
            && accountId < STAGE_CAST_ACCOUNT_END;
    }

    bool IsCitizenAccount(uint32 accountId)
    {
        if (IsStageCastAccount(accountId))
            return true;

        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT 1 FROM playerbots_account_type WHERE account_id = {} AND account_type = {}",
            accountId, CITIZEN_ACCOUNT_TYPE);
        return result != nullptr;
    }

    uint32 CountCitizenCharacters()
    {
        if (!CitizenRosterRegistry::Instance().IsLoaded())
            return 0;
        return static_cast<uint32>(CitizenRosterRegistry::Instance().All().size());
    }

    void ValidateStageCast()
    {
        if (_validated)
            return;
        _validated = true;

        CitizenRosterRegistry::Instance().LoadFromDatabase();
        bool const requireListed =
            sConfigMgr->GetOption<bool>("CitizenBots.Assign.RequireListed", true, false);
        if (!CitizenRosterRegistry::Instance().IsLoaded())
        {
            uint32 const tableRows = CitizenRosterRegistry::Instance().TableRowCount();
            if (tableRows == 0)
            {
                CbLog::Error("citizen_roster table empty or missing — the core updater never applies "
                             "data/sql/playerbots; import " CB_ROSTER_SQL " by hand, then restart");
            }
            else
            {
                // The table is there; every row was dropped by the table's enabled flag or by
                // CitizenBots.Assign.* with RequireListed=1. A re-import will not change that.
                CbLog::Error("citizen_roster has {} rows but 0 active city bots — {} disabled in the table, "
                             "{} not listed or refused by CitizenBots.Assign.* (RequireListed={}). "
                             "Check mod_city_bots.conf; a re-import will not help.",
                             tableRows,
                             CitizenRosterRegistry::Instance().DisabledCount(),
                             CitizenRosterRegistry::Instance().UnlistedCount(),
                             requireListed ? 1 : 0);
            }
            return;
        }

        uint32 missingChars = 0;
        uint32 identityMismatches = 0;

        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(entry.guid);
            uint32 const accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
            if (!accountId)
            {
                ++missingChars;
                continue;
            }

            std::string characterName;
            sCharacterCache->GetCharacterNameByGuid(guid, characterName);
            if (accountId != entry.accountId || characterName != entry.name)
                ++identityMismatches;
        }

        uint32 const rosterSize =
            static_cast<uint32>(CitizenRosterRegistry::Instance().All().size());

        if (rosterSize < STAGE_CAST_CAPACITY)
        {
            uint32 const tableRows = CitizenRosterRegistry::Instance().TableRowCount();
            if (tableRows < STAGE_CAST_CAPACITY)
            {
                CbLog::Error("citizen_roster has {}/{} rows ({} active) — re-import "
                             CB_ROSTER_SQL " then restart",
                             tableRows, STAGE_CAST_CAPACITY, rosterSize);
            }
            else
            {
                // The table is complete; the shortfall is config, not a missing import.
                CbLog::Info("stage cast roster: {}/{} entries active — {} disabled in citizen_roster, "
                            "{} not listed or refused by CitizenBots.Assign.* (RequireListed={}). "
                            "Not a missing import.",
                            rosterSize, STAGE_CAST_CAPACITY,
                            CitizenRosterRegistry::Instance().DisabledCount(),
                            CitizenRosterRegistry::Instance().UnlistedCount(),
                            requireListed ? 1 : 0);
            }
        }

        if (missingChars)
        {
            CbLog::Error("{} stage cast characters missing from acore_characters — "
                         "run db-characters update " CB_CHARS_SQL,
                         missingChars);
        }

        if (identityMismatches)
        {
            CbLog::Error(
                "{} stage-cast characters have account/name identity different from citizen_roster. "
                "Player::LoadFromDB can reject or strand these logins. Re-import " CB_ROSTER_SQL
                " (shipped roster), or if your roster is intentionally customized apply "
                CB_ROSTER_RESYNC_SQL " to acore_characters",
                identityMismatches);
        }

        QueryResult badAtLogin = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM characters WHERE guid >= {} AND guid < {} "
            "AND at_login <> 0",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (badAtLogin)
        {
            uint32 const flagged = (*badAtLogin)[0].Get<uint32>();
            if (flagged)
            {
                CbLog::Warn(
                    "{} stage-cast characters have non-zero at_login — can block bot login. "
                    "Re-apply " CB_CHARS_SQL " (re-seeds all 400 stage-cast characters)",
                    flagged);
            }
        }

        QueryResult invalidCharacterNames = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM characters WHERE guid >= {} AND guid < {} "
            "AND (CHAR_LENGTH(name) < 2 OR CHAR_LENGTH(name) > 12 "
            "OR name REGEXP '[^A-Za-z]')",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (invalidCharacterNames)
        {
            uint32 const invalid = (*invalidCharacterNames)[0].Get<uint32>();
            if (invalid)
            {
                CbLog::Error(
                    "{} stage-cast characters have names that fail core player-name validation. "
                    "Player::LoadFromDB rejects these before the bot reaches the world. "
                    "Run db-characters update " CB_CHARS_SQL,
                    invalid);
            }
        }

        QueryResult invalidRosterNames = PlayerbotsDatabase.Query(
            "SELECT COUNT(*) FROM citizen_roster WHERE guid >= {} AND guid < {} "
            "AND (CHAR_LENGTH(character_name) < 2 OR CHAR_LENGTH(character_name) > 12 "
            "OR character_name REGEXP '[^A-Za-z]')",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (invalidRosterNames)
        {
            uint32 const invalid = (*invalidRosterNames)[0].Get<uint32>();
            if (invalid)
            {
                CbLog::Error(
                    "{} citizen_roster entries have invalid player names. "
                    "Re-import " CB_ROSTER_SQL,
                    invalid);
            }
        }

        QueryResult missingHomebind = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM characters c "
            "LEFT JOIN character_homebind h ON h.guid = c.guid "
            "WHERE c.guid >= {} AND c.guid < {} AND h.guid IS NULL",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (missingHomebind)
        {
            uint32 const missing = (*missingHomebind)[0].Get<uint32>();
            if (missing)
            {
                CbLog::Error(
                    "{} stage-cast characters are missing character_homebind rows. "
                    "Player::LoadFromDB hard-fails at this boundary, causing the fixed 51-online cap. "
                    "Run db-characters update " CB_CHARS_SQL,
                    missing);
            }
        }

        QueryResult missingSkills = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM characters c "
            "LEFT JOIN (SELECT DISTINCT guid FROM character_skills) s ON s.guid = c.guid "
            "WHERE c.guid >= {} AND c.guid < {} AND s.guid IS NULL",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (missingSkills)
        {
            uint32 const missing = (*missingSkills)[0].Get<uint32>();
            if (missing)
            {
                CbLog::Warn(
                    "{} stage-cast characters are missing character_skills rows. "
                    "Core regenerates default skills after a successful login; "
                    "no action needed on a fresh install.",
                    missing);
            }
        }

        QueryResult missingReputation = CharacterDatabase.Query(
            "SELECT COUNT(*) FROM characters c "
            "LEFT JOIN (SELECT DISTINCT guid FROM character_reputation) r ON r.guid = c.guid "
            "WHERE c.guid >= {} AND c.guid < {} AND r.guid IS NULL",
            STAGE_CAST_GUID_START, STAGE_CAST_GUID_END);
        if (missingReputation)
        {
            uint32 const missing = (*missingReputation)[0].Get<uint32>();
            if (missing)
            {
                CbLog::Warn(
                    "{} stage-cast characters are missing character_reputation rows. "
                    "Core regenerates them after a successful login; no action needed on a fresh install.",
                    missing);
            }
        }

        QueryResult wrongType = PlayerbotsDatabase.Query(
            "SELECT COUNT(*) FROM playerbots_account_type "
            "WHERE account_id BETWEEN {} AND {} AND account_type != {}",
            STAGE_CAST_ACCOUNT_START, STAGE_CAST_ACCOUNT_END - 1, CITIZEN_ACCOUNT_TYPE);
        if (wrongType)
        {
            uint32 const bad = (*wrongType)[0].Get<uint32>();
            if (bad)
            {
                CbLog::Error(
                    "{} stage-cast accounts have wrong playerbots account_type (need {}, not 2/AddClass) — "
                    "run on acore_playerbots: mysql -u acore -p acore_playerbots < "
                    "modules/mod-city-bots/data/sql/" CB_ROSTER_SQL,
                    bad, CITIZEN_ACCOUNT_TYPE);
            }
        }

        CbLog::Info("stage cast loaded: {}/{} roster entries | accounts {}-{} | "
                    "GUIDs {}-{} | password: citybot_stage",
                    rosterSize, STAGE_CAST_CAPACITY,
                    STAGE_CAST_ACCOUNT_START, STAGE_CAST_ACCOUNT_END - 1,
                    STAGE_CAST_GUID_START, STAGE_CAST_GUID_END - 1);
    }
}
