#include "CbPlayerbotsIntegration.h"

#include "CbLog.h"
#include "CbPlayerbotHolderAccess.h"
#include "CitizenRosterRegistry.h"
#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "RandomPlayerbotMgr.h"
#include "StringFormat.h"
#include "WorldSessionMgr.h"

namespace CbPlayerbotsIntegration
{
    uint32 CountBotLoading()
    {
        return CbPlayerbotHolderAccess::CountBotLoading();
    }

    bool IsGuidInBotLoading(uint32 guidLow)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        return CbPlayerbotHolderAccess::IsGuidInBotLoading(guid);
    }

    void ReleaseBotLoading(uint32 guidLow)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        CbPlayerbotHolderAccess::ReleaseBotLoading(guid);
    }

    uint32 RemoveFromRandomPlayerList(uint32 guidLow)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        return CbPlayerbotHolderAccess::RemoveFromPlayerbots(sRandomPlayerbotMgr, guid);
    }

    uint32 RemoveStageCastFromRandomPlayerList()
    {
        uint32 removed = 0;
        if (!CitizenRosterRegistry::Instance().IsLoaded())
            return removed;

        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
            removed += RemoveFromRandomPlayerList(entry.guid);

        return removed;
    }

    std::size_t CharacterDatabaseQueueSize()
    {
        return CharacterDatabase.QueueSize();
    }

    StageCastLoginResult TryLoginStageCastBot(uint32 guidLow, std::string& detail)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);

        Player* online = sRandomPlayerbotMgr.GetPlayerBot(guid);
        if (online && online->IsInWorld())
        {
            detail = "already online";
            return StageCastLoginResult::AlreadyOnline;
        }

        if (IsGuidInBotLoading(guidLow))
        {
            detail = "already in botLoading";
            return StageCastLoginResult::AlreadyLoading;
        }

        Player* connected = ObjectAccessor::FindConnectedPlayer(guid);
        if (connected)
        {
            if (connected->IsInWorld() && connected->GetSession() &&
                connected->GetSession()->IsBot())
            {
                RemoveFromRandomPlayerList(guidLow);
                detail = "already online outside random map";
                return StageCastLoginResult::AlreadyOnline;
            }

            if (connected->isAFK())
                connected->ToggleAFK();    // crowd-scene AFK must not survive logout
            sRandomPlayerbotMgr.LogoutPlayerBot(guid);
            detail = "connected zombie session logged out";
            return StageCastLoginResult::ZombieLogout;
        }

        uint32 const accountId = sCharacterCache->GetCharacterAccountIdByGuid(guid);
        if (!accountId)
        {
            detail = "character missing from CharacterCache";
            return StageCastLoginResult::NoAccount;
        }

        if (CitizenRosterRegistry::Instance().IsLoaded())
        {
            if (CitizenRosterEntry const* entry = CitizenRosterRegistry::Instance().FindByGuid(guidLow))
            {
                std::string characterName;
                sCharacterCache->GetCharacterNameByGuid(guid, characterName);

                if (accountId != entry->accountId || characterName != entry->name)
                {
                    detail = Acore::StringFormat(
                        "character identity mismatch (characters account/name {} '{}' vs citizen_roster {} '{}'); "
                        "apply db-characters update 2026_07_25_02_sync_stage_cast_identity_to_roster.sql",
                        accountId, characterName, entry->accountId, entry->name);
                    return StageCastLoginResult::Failed;
                }
            }
        }

        std::size_t const charDbQueue = CharacterDatabaseQueueSize();
        uint32 const sessionCount = sWorldSessionMgr->GetPlayerCount();
        uint32 const sessionLimit = sWorldSessionMgr->GetPlayerAmountLimit();
        bool const addClassBot = sRandomPlayerbotMgr.IsAddclassBot(guidLow);

        sRandomPlayerbotMgr.AddPlayerBot(guid, 0);

        if (IsGuidInBotLoading(guidLow))
        {
            detail = Acore::StringFormat(
                "queued (account {}, addclass={}, charDB={}, sessions {}/{})",
                accountId, addClassBot ? 1 : 0, charDbQueue, sessionCount, sessionLimit);
            return StageCastLoginResult::AlreadyLoading;
        }

        detail = Acore::StringFormat(
            "AddPlayerBot no-op (account {}, addclass={}, charDB={}, sessions {}/{}, "
            "playerbots {}, botLoading {})",
            accountId, addClassBot ? 1 : 0, charDbQueue, sessionCount, sessionLimit,
            sRandomPlayerbotMgr.GetPlayerbotsCount(), CountBotLoading());

        if (sessionLimit > 0 && sessionCount >= sessionLimit)
        {
            CbLog::Error(
                "stage-cast login blocked by worldserver PlayerLimit ({} >= {}). "
                "Raise PlayerLimit to at least MaxRandomBots + 135 (e.g. 1335 for 1200 random + 135 city).",
                sessionCount, sessionLimit);
        }

        return StageCastLoginResult::Failed;
    }
}
