#include "CbCitizenLoginMgr.h"

#include "CbCitizenAccountMgr.h"
#include "CbPlayerbotsIntegration.h"
#include "CitizenRosterRegistry.h"
#include "CityBotsRuntime.h"

#include "Ai/City/CityBots/CitizenInfo.h"

#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "CityPopulationMgr.h"
#include "CbDuelBotUtil.h"

#include "CharacterDatabase.h"
#include "DatabaseEnv.h"
#include "CbLog.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "StringFormat.h"
#include "WorldSessionMgr.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    char const* const kCitizenEvent = "citizen";
    char const* const kBurstEvent = "citizen_burst";
    uint32 _loginAccumMs = 0;
    uint32 _statusAccumMs = 0;
    uint32 _waitStatusAccumMs = 0;
    constexpr uint32 LOGIN_INTERVAL_MS = 500;
    constexpr uint32 STATUS_INTERVAL_MS = 60 * 1000;
    constexpr uint32 WAIT_STATUS_INTERVAL_MS = 30 * 1000;
    constexpr uint32 LOGIN_RETRY_COOLDOWN_MS = 5 * IN_MILLISECONDS;
    constexpr uint32 LOGIN_ATTEMPT_GRACE_MS = 45 * IN_MILLISECONDS;
    constexpr uint32 MAX_CONCURRENT_STAGE_LOGINS = 32;
    constexpr std::size_t CHAR_DB_QUEUE_PAUSE = 512;
    constexpr uint32 INTEGRATION_INTERVAL_MS = 5 * IN_MILLISECONDS;
    bool _poolPrepared = false;
    bool _randomLoginComplete = false;
    bool _characterDbReady = false;
    uint32 _integrationAccumMs = 0;
    uint32 _dbWaitStatusAccumMs = 0;
    std::unordered_map<uint32, uint32> _loginBlockedAt;
    std::unordered_map<uint32, uint32> _loginAttemptAt;
    std::unordered_map<uint32, std::string> _loginAttemptDetail;

    void NoteRosterBotLoggedIn(uint32 guidLow);

    void ClearLoginAttempt(uint32 guidLow)
    {
        _loginAttemptAt.erase(guidLow);
        _loginAttemptDetail.erase(guidLow);
    }

    void MarkLoginAttempt(uint32 guidLow, std::string const& detail)
    {
        _loginAttemptAt[guidLow] = getMSTime();
        _loginAttemptDetail[guidLow] = detail;
    }

    void ClearCitizenLoginFlags(uint32 guidLow)
    {
        sRandomPlayerbotMgr.SetValue(guidLow, "add", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "logout", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, kCitizenEvent, 0);
        sRandomPlayerbotMgr.SetValue(guidLow, kBurstEvent, 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "randomize", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "teleport", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "update", 0);
    }

    void AbortCitizenLoginAttempt(uint32 guidLow, char const* reason)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        if (Player* stuck = sRandomPlayerbotMgr.GetPlayerBot(guid))
        {
            if (stuck->isAFK())
                stuck->ToggleAFK();    // crowd-scene AFK must not survive logout
            sRandomPlayerbotMgr.LogoutPlayerBot(guid);
        }

        CbPlayerbotsIntegration::ReleaseBotLoading(guidLow);
        ClearCitizenLoginFlags(guidLow);
        ClearLoginAttempt(guidLow);
        sRandomPlayerbotMgr.OnPlayerLoginError(guidLow);
        _loginBlockedAt[guidLow] = getMSTime();
        CbLog::Warn("stage-cast login aborted for guid {} ({})", guidLow, reason);
    }

    bool IsLoginInFlight(uint32 guidLow)
    {
        return CbPlayerbotsIntegration::IsGuidInBotLoading(guidLow);
    }

    bool IsRosterOnline(uint32 guid)
    {
        ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(guid);
        Player* bot = ObjectAccessor::FindConnectedPlayer(playerGuid);
        return bot && bot->IsInWorld() && bot->GetSession() && bot->GetSession()->IsBot();
    }

    bool IsLoginPending(uint32 guidLow)
    {
        if (IsLoginInFlight(guidLow))
            return true;

        if (IsRosterOnline(guidLow))
            return false;

        auto it = _loginAttemptAt.find(guidLow);
        if (it == _loginAttemptAt.end())
            return false;

        return GetMSTimeDiffToNow(it->second) < LOGIN_ATTEMPT_GRACE_MS;
    }

    bool HasLoginCooldown(uint32 guidLow)
    {
        auto it = _loginBlockedAt.find(guidLow);
        if (it == _loginBlockedAt.end())
            return false;

        return GetMSTimeDiffToNow(it->second) < LOGIN_RETRY_COOLDOWN_MS;
    }

    void MarkCitizenBot(uint32 guidLow, bool burst)
    {
        sRandomPlayerbotMgr.SetValue(guidLow, "logout", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, kCitizenEvent, 1);
        sRandomPlayerbotMgr.SetValue(guidLow, kBurstEvent, burst ? 1 : 0);
        // Stage cast identity comes from citizen_roster/characters and must not be rerolled.
        sRandomPlayerbotMgr.SetValue(guidLow, "randomize", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "teleport", 0);
        sRandomPlayerbotMgr.SetValue(guidLow, "update", 0);
    }

    uint32 CountPendingCitizenLogins()
    {
        uint32 count = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (IsRosterOnline(entry.guid))
                continue;
            if (IsLoginPending(entry.guid))
                ++count;
        }
        return count;
    }

    uint32 CountPendingCitizenLoginsAtHome(char const* homeKey)
    {
        uint32 count = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (entry.homeKey != homeKey)
                continue;
            if (IsRosterOnline(entry.guid))
                continue;
            if (IsLoginPending(entry.guid))
                ++count;
        }
        return count;
    }

    void ReconcileOnlineCitizenLogins()
    {
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(entry.guid);

            if (IsRosterOnline(entry.guid))
            {
                _loginBlockedAt.erase(entry.guid);
                ClearLoginAttempt(entry.guid);
                CbPlayerbotsIntegration::RemoveFromRandomPlayerList(entry.guid);
                continue;
            }

            Player* connected = ObjectAccessor::FindConnectedPlayer(guid);
            if (!connected || !connected->IsInWorld())
                continue;

            if (false && sRandomPlayerbotMgr.GetPlayerBot(guid) != connected)
            {
                sRandomPlayerbotMgr.OnBotLogin(connected);
                CbLog::Warn(
                    "stage-cast {} (guid {}) in world but not in playerbots map — "
                    "registered via reconcile",
                    entry.name, entry.guid);
            }

            NoteRosterBotLoggedIn(entry.guid);
        }
    }

    uint32 CountStageOnlineAtHome(char const* homeKey)
    {
        uint32 count = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (entry.homeKey != homeKey)
                continue;
            if (!IsRosterOnline(entry.guid))
                continue;
            CityLocationDef const* loc =
                CityLocationRegistry::Instance().FindByKey(homeKey);
            if (loc && loc->duelHub)
            {
                ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(entry.guid);
                Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
                if (!bot || !CbDuelBotUtil::IsEligible(bot))
                    continue;
            }
            ++count;
        }
        return count;
    }

    uint32 StageTargetOnline()
    {
        uint32 sum = 0;
        for (CityLocationDef const& loc : CityLocationRegistry::Instance().All())
        {
            uint32 desired = CityLocationRegistry::Instance().GetDesiredCount(loc.key);
            uint32 roster = CitizenRosterRegistry::Instance().CountRosterAtHome(loc.key);
            uint32 staticRoster = CitizenRosterRegistry::Instance().CountStaticRosterAtHome(loc.key);
            if (!desired || !roster)
                continue;
            sum += std::min(std::max(desired, staticRoster), roster);
        }
        return sum;
    }

    bool WantsRosterEntry(CitizenRosterEntry const& entry)
    {
        uint32 configured = CityLocationRegistry::Instance().GetConfiguredCount(entry.homeKey.c_str());
        if (!configured)
            return false;

        if (!entry.burstOnly)
            return true;

        uint32 desired = CityLocationRegistry::Instance().GetDesiredCount(entry.homeKey.c_str());
        if (!desired)
            return false;

        uint32 staticRoster =
            CitizenRosterRegistry::Instance().CountStaticRosterAtHome(entry.homeKey.c_str());
        uint32 online = CountStageOnlineAtHome(entry.homeKey.c_str());
        uint32 pending = CountPendingCitizenLoginsAtHome(entry.homeKey.c_str());
        uint32 cap = std::min(std::max(desired, staticRoster),
                              CitizenRosterRegistry::Instance().CountRosterAtHome(
                                  entry.homeKey.c_str()));
        return online + pending < cap;
    }

    uint32 CountWantedRosterLoginSlots()
    {
        uint32 count = 0;
        for (CitizenRosterEntry const* entryPtr : CitizenRosterRegistry::Instance().LoginOrder())
        {
            CitizenRosterEntry const& entry = *entryPtr;
            if (IsRosterOnline(entry.guid))
                continue;
            if (HasLoginCooldown(entry.guid))
                continue;
            if (IsLoginPending(entry.guid))
                continue;
            if (!WantsRosterEntry(entry))
                continue;
            ++count;
        }
        return count;
    }

    void RecoverStuckLoginAttempts()
    {
        uint32 reset = 0;
        std::string examples;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (IsRosterOnline(entry.guid))
            {
                ClearLoginAttempt(entry.guid);
                continue;
            }

            if (IsLoginInFlight(entry.guid))
            {
                Player* connected = ObjectAccessor::FindConnectedPlayer(
                    ObjectGuid::Create<HighGuid::Player>(entry.guid));
                if (connected && connected->IsInWorld() && connected->GetSession() &&
                    connected->GetSession()->IsBot())
                {
                    NoteRosterBotLoggedIn(entry.guid);
                    continue;
                }

                continue;
            }

            auto attemptIt = _loginAttemptAt.find(entry.guid);
            if (attemptIt == _loginAttemptAt.end())
                continue;

            uint32 const elapsed = GetMSTimeDiffToNow(attemptIt->second);
            if (elapsed < LOGIN_ATTEMPT_GRACE_MS)
                continue;

            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(entry.guid);
            Player* connected = ObjectAccessor::FindConnectedPlayer(guid);
            if (connected && connected->IsInWorld())
            {
                NoteRosterBotLoggedIn(entry.guid);
                continue;

                if (sRandomPlayerbotMgr.GetPlayerBot(guid) != connected)
                {
                    sRandomPlayerbotMgr.OnBotLogin(connected);
                    CbLog::Warn(
                        "stage-cast {} (guid {}) login finished in world without playerbots "
                        "registration after {}ms — recovered",
                        entry.name, entry.guid, elapsed);
                }
                NoteRosterBotLoggedIn(entry.guid);
                continue;
            }

            std::string detail = _loginAttemptDetail[entry.guid];
            std::string reason = Acore::StringFormat(
                "Player::LoadFromDB failed after {}ms ({})", elapsed, detail);
            AbortCitizenLoginAttempt(entry.guid, reason.c_str());
            if (reset < 5)
            {
                if (!examples.empty())
                    examples += ", ";
                examples += entry.name + "/" + entry.homeKey;
            }
            ++reset;
        }

        if (reset)
            CbLog::Info("cleared {} stuck stage-cast login attempts: {}", reset, examples);
    }

    void ResetStuckCitizenFlags()
    {
        RecoverStuckLoginAttempts();
    }

    void DisableStageCastRandomMaintenance()
    {
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            sRandomPlayerbotMgr.SetValue(entry.guid, "randomize", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "teleport", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "update", 0);
        }
    }

    void ClearStageCastLoginQueue()
    {
        uint32 cleared = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (IsRosterOnline(entry.guid))
                continue;

            if (sRandomPlayerbotMgr.GetValue(entry.guid, "add") ||
                sRandomPlayerbotMgr.GetValue(entry.guid, "logout") ||
                sRandomPlayerbotMgr.GetValue(entry.guid, kCitizenEvent) ||
                sRandomPlayerbotMgr.GetValue(entry.guid, kBurstEvent))
                ++cleared;

            sRandomPlayerbotMgr.SetValue(entry.guid, "add", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "logout", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, kCitizenEvent, 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, kBurstEvent, 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "randomize", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "teleport", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "update", 0);
            _loginBlockedAt.erase(entry.guid);
            ClearLoginAttempt(entry.guid);
        }

        if (cleared)
            CbLog::Info("cleared {} saved stage-cast login queue flags at startup", cleared);
    }

    uint32 CountOnlinePlayerbotsExcludingCitizens()
    {
        uint32 count = 0;
        for (auto const& [guid, player] : sRandomPlayerbotMgr.GetAllBots())
        {
            if (!player || !player->IsInWorld())
                continue;
            if (!player->GetSession() || !player->GetSession()->IsBot())
                continue;
            if (CitizenRosterRegistry::Instance().IsRosterGuid(guid.GetCounter()))
                continue;
            ++count;
        }
        return count;
    }

    bool IsRandomAutologinComplete(uint32 /*diff*/)
    {
        if (_randomLoginComplete)
            return true;

        if (!sPlayerbotAIConfig.randomBotAutologin)
        {
            _randomLoginComplete = true;
            return true;
        }

        uint32 const online = CountOnlinePlayerbotsExcludingCitizens();
        uint32 const target = sRandomPlayerbotMgr.GetMaxAllowedBotCount();
        if (!target)
            return false;

        if (online >= target)
        {
            _randomLoginComplete = true;
            CbLog::Info(
                "playerbots autologin complete ({}/{} non-city bots), waiting for character DB queue "
                "before stage cast login (queue {})",
                online, target, CbPlayerbotsIntegration::CharacterDatabaseQueueSize());
            return true;
        }

        return false;
    }

    bool IsCharacterDatabaseReadyForStageCast(uint32 diff)
    {
        if (_characterDbReady)
            return true;

        std::size_t const queueSize = CbPlayerbotsIntegration::CharacterDatabaseQueueSize();
        if (queueSize > CHAR_DB_QUEUE_PAUSE)
        {
            _dbWaitStatusAccumMs += diff;
            if (_dbWaitStatusAccumMs >= WAIT_STATUS_INTERVAL_MS)
            {
                _dbWaitStatusAccumMs = 0;
                CbLog::Info("waiting for character DB queue before stage cast "
                            "(queue {}, need <= {})",
                            queueSize, CHAR_DB_QUEUE_PAUSE);
            }
            return false;
        }

        _characterDbReady = true;
        CbLog::Info("character DB queue ready for stage cast (queue {})", queueSize);
        return true;
    }

    void LogPeriodicStatus()
    {
        auto& locReg = CityLocationRegistry::Instance();
        uint32 target = StageTargetOnline();
        uint32 online = CbCitizenLoginMgr::CountOnlineCitizens();

        uint32 const rosterSize =
            static_cast<uint32>(CitizenRosterRegistry::Instance().All().size());

        auto desiredAt = [](char const* key) -> uint32
        {
            return std::min(CityLocationRegistry::Instance().GetDesiredCount(key),
                            CitizenRosterRegistry::Instance().CountRosterAtHome(key));
        };

        std::string locationStatus;
        for (CityLocationDef const& loc : locReg.All())
        {
            if (!locationStatus.empty())
                locationStatus += " | ";

            uint32 const homeOnline = CountStageOnlineAtHome(loc.key);
            uint32 const desired = desiredAt(loc.key);
            uint32 const present = locReg.CountPresentAtLocation(loc.key);
            uint32 const roster = CitizenRosterRegistry::Instance().CountRosterAtHome(loc.key);
            uint32 const minCount = locReg.GetMinCount(loc.key);
            uint32 const maxCount = locReg.GetConfiguredCount(loc.key);

            locationStatus += Acore::StringFormat(
                "{} online={} present={} desired={} min={} max={} roster={}",
                loc.key, homeOnline, present, desired, minCount, maxCount, roster);
        }

        CbLog::Info("status: city-online={}/{} ({} in-flight, {} loading, desired-total={}) | "
                    "roster {}/{} | charDB={} | {}",
                    online, rosterSize, CountPendingCitizenLogins(),
                    CbPlayerbotsIntegration::CountBotLoading(), target,
                    rosterSize, STAGE_CAST_CAPACITY,
                    CbPlayerbotsIntegration::CharacterDatabaseQueueSize(),
                    locationStatus);
    }

    void NoteRosterBotLoggedIn(uint32 guidLow)
    {
        _loginBlockedAt.erase(guidLow);
        ClearLoginAttempt(guidLow);
        CbPlayerbotsIntegration::RemoveFromRandomPlayerList(guidLow);
    }
}

namespace CbCitizenLoginMgr
{
    bool UsesDedicatedPool()
    {
        return CitizenRosterRegistry::Instance().IsLoaded();
    }

    void PrepareDedicatedPool()
    {
        if (_poolPrepared)
            return;
        _poolPrepared = true;

        CbCitizenAccountMgr::ValidateStageCast();
        CitizenRosterRegistry::Instance().LoadFromDatabase();
        DisableStageCastRandomMaintenance();
        ClearStageCastLoginQueue();
        ResetStuckCitizenFlags();

        uint32 const sessionLimit = sWorldSessionMgr->GetPlayerAmountLimit();
        uint32 const randomTarget = sRandomPlayerbotMgr.GetMaxAllowedBotCount();
        uint32 const cityTarget = StageTargetOnline();
        if (sessionLimit > 0 && sessionLimit < randomTarget + cityTarget)
        {
            CbLog::Error(
                "worldserver PlayerLimit ({}) is below MaxRandomBots + city target ({} + {} = {}). "
                "City logins will fail once random bots finish. Set PlayerLimit >= {} in worldserver.conf.",
                sessionLimit, randomTarget, cityTarget, randomTarget + cityTarget,
                randomTarget + cityTarget);
        }

        CbLog::Info("stage cast login pool: {} roster entries, target {} online",
                    CitizenRosterRegistry::Instance().All().size(),
                    StageTargetOnline());
    }

    uint32 CountOnlineCitizens()
    {
        uint32 count = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(entry.guid);
            Player* player = ObjectAccessor::FindConnectedPlayer(guid);
            if (!player || !player->IsInWorld())
                continue;
            if (!player->GetSession() || !player->GetSession()->IsBot())
                continue;
            ++count;
        }
        return count;
    }

    void RestoreCitizensToCurrentBots()
    {
        if (!UsesDedicatedPool())
            return;

        uint32 cleared = 0;
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            if (!sRandomPlayerbotMgr.GetValue(entry.guid, "add"))
                continue;

            sRandomPlayerbotMgr.SetValue(entry.guid, "add", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "logout", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, kCitizenEvent, 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, kBurstEvent, 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "randomize", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "teleport", 0);
            sRandomPlayerbotMgr.SetValue(entry.guid, "update", 0);
            ++cleared;
        }

        if (cleared)
            CbLog::Info("discarded {} stale playerbots add events for stage-cast bots", cleared);
    }

    void Tick(uint32 diff)
    {
        if (!CityBotsRuntime::ContextsRegistered())
            return;

        if (!sPlayerbotAIConfig.enabled || !sPlayerbotAIConfig.randomBotAutologin)
            return;

        PrepareDedicatedPool();
        if (!UsesDedicatedPool())
            return;

        if (!IsRandomAutologinComplete(diff))
        {
            _waitStatusAccumMs += diff;
            if (_waitStatusAccumMs >= WAIT_STATUS_INTERVAL_MS)
            {
                _waitStatusAccumMs = 0;
                uint32 const online = CountOnlinePlayerbotsExcludingCitizens();
                uint32 const target = sRandomPlayerbotMgr.GetMaxAllowedBotCount();
                CbLog::Info("waiting for playerbots autologin ({}/{})", online, target);
            }
            return;
        }

        if (!IsCharacterDatabaseReadyForStageCast(diff))
            return;

        ReconcileOnlineCitizenLogins();

        CityPopulationMgr::Tick(diff);

        _integrationAccumMs += diff;
        if (_integrationAccumMs >= INTEGRATION_INTERVAL_MS)
        {
            _integrationAccumMs = 0;
            CbPlayerbotsIntegration::RemoveStageCastFromRandomPlayerList();
        }

        _statusAccumMs += diff;
        if (_statusAccumMs >= STATUS_INTERVAL_MS)
        {
            _statusAccumMs = 0;
            LogPeriodicStatus();
        }

        _loginAccumMs += diff;
        if (_loginAccumMs < LOGIN_INTERVAL_MS)
            return;
        _loginAccumMs = 0;

        uint32 const globalLoading = CbPlayerbotsIntegration::CountBotLoading();
        RecoverStuckLoginAttempts();

        uint32 target = StageTargetOnline();
        uint32 online = CountOnlineCitizens();
        uint32 pendingBefore = CountPendingCitizenLogins();

        uint32 wantedSlots = CountWantedRosterLoginSlots();
        if (!wantedSlots)
            return;

        uint32 const toLogin =
            std::min(MAX_CONCURRENT_STAGE_LOGINS, wantedSlots);
        if (!toLogin)
            return;

        uint32 started = 0;
        uint32 failed = 0;
        uint32 skippedPending = 0;

        for (CitizenRosterEntry const* entryPtr : CitizenRosterRegistry::Instance().LoginOrder())
        {
            if (started >= toLogin)
                break;

            CitizenRosterEntry const& entry = *entryPtr;
            if (IsRosterOnline(entry.guid))
                continue;
            if (HasLoginCooldown(entry.guid))
                continue;
            if (IsLoginPending(entry.guid))
            {
                ++skippedPending;
                continue;
            }
            if (!WantsRosterEntry(entry))
                continue;

            CityLocationRegistry::Instance().StoreHomeLocation(entry.guid, entry.homeKey);
            MarkCitizenBot(entry.guid, entry.burstOnly);

            bool const firstAttempt = !_loginAttemptAt.count(entry.guid);
            std::string detail;
            auto const result =
                CbPlayerbotsIntegration::TryLoginStageCastBot(entry.guid, detail);

            switch (result)
            {
                case CbPlayerbotsIntegration::StageCastLoginResult::AlreadyOnline:
                    NoteRosterBotLoggedIn(entry.guid);
                    CbLog::Info("stage-cast {} (guid {}) already online -> {}",
                                entry.name, entry.guid, entry.homeKey);
                    break;
                case CbPlayerbotsIntegration::StageCastLoginResult::AlreadyLoading:
                    MarkLoginAttempt(entry.guid, detail);
                    ++started;
                    if (firstAttempt)
                    {
                        CbLog::Info("stage-cast {} (guid {}) -> {} [{}]",
                                    entry.name, entry.guid, entry.homeKey, detail);
                    }
                    break;
                case CbPlayerbotsIntegration::StageCastLoginResult::ZombieLogout:
                    CbLog::Warn("stage-cast {} (guid {}) -> {} [zombie: {}]",
                                entry.name, entry.guid, entry.homeKey, detail);
                    break;
                case CbPlayerbotsIntegration::StageCastLoginResult::NoAccount:
                case CbPlayerbotsIntegration::StageCastLoginResult::Failed:
                    ++failed;
                    _loginBlockedAt[entry.guid] = getMSTime();
                    CbLog::Warn("stage-cast {} (guid {}) -> {} [{}]",
                                entry.name, entry.guid, entry.homeKey, detail);
                    break;
                default:
                    break;
            }
        }

        uint32 const loadingAfter = CbPlayerbotsIntegration::CountBotLoading();
        uint32 const pendingAfter = CountPendingCitizenLogins();
        if (started || failed || skippedPending)
        {
            CbLog::Info(
                "stage cast login tick: {} started, {} failed, {} skipped-pending, "
                "{} online / {} target | botLoading {}->{} pending {}->{} charDB={}",
                started, failed, skippedPending, online, target,
                globalLoading, loadingAfter, pendingBefore, pendingAfter,
                CbPlayerbotsIntegration::CharacterDatabaseQueueSize());
        }
    }

    void OnRosterBotLoggedIn(uint32 guidLow)
    {
        NoteRosterBotLoggedIn(guidLow);
    }
}
