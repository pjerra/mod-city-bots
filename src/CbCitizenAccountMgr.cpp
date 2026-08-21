#include "CbCitizenAccountMgr.h"

#include "Ai/City/CityBots/CitizenInfo.h"
#include "CitizenRosterRegistry.h"

#include "AccountMgr.h"
#include "CbLog.h"
#include "CharacterCache.h"
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
        if (!CitizenRosterRegistry::Instance().IsLoaded())
        {
            CbLog::Error("citizen_roster table empty — apply V2 SQL updates "
                         "(db-auth, db-characters, playerbots) then restart");
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
            CbLog::Error("stage cast roster has {}/{} entries — apply "
                         "2026_07_15_02_scale_stage_cast_400.sql updates "
                         "(auth, characters, playerbots) then restart",
                         rosterSize, STAGE_CAST_CAPACITY);
        }

        if (missingChars)
        {
            CbLog::Error("{} stage cast characters missing from acore_characters — "
                         "run db-characters update 2026_07_15_02_scale_stage_cast_400.sql",
                         missingChars);
        }

        if (identityMismatches)
        {
            CbLog::Error(
                "{} stage-cast characters have account/name identity different from citizen_roster. "
                "Player::LoadFromDB can reject or strand these logins. Apply db-characters update "
                "2026_07_25_02_sync_stage_cast_identity_to_roster.sql",
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
                    "Apply db-characters update 2026_07_17_03_fix_stage_cast_invalid_names.sql",
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
                    "Run db-characters update 2026_07_17_03_fix_stage_cast_invalid_names.sql",
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
                    "Run playerbots update 2026_07_17_03_fix_stage_cast_invalid_names.sql",
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
                    "Run db-characters update 2026_07_17_01_fix_stage_cast_required_login_rows.sql",
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
                    "Core can regenerate default skills after a successful login, but "
                    "applying the backfill keeps first login deterministic. "
                    "Run db-characters update 2026_07_17_02_backfill_stage_cast_skills_reputation.sql",
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
                    "Run db-characters update 2026_07_17_02_backfill_stage_cast_skills_reputation.sql",
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
                    "run on acore_playerbots: "
                    "mysql -u acore -p acore_playerbots < "
                    "modules/mod-city-bots/data/sql/playerbots/updates/"
                    "2026_07_16_12_restore_stage_cast_account_type_3.sql",
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
