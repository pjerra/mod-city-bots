#include "CityPopulationMgr.h"

#include "Ai/City/CityBots/Data/CityHubRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "CbLog.h"

#include "DBCStores.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Timer.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    enum class HubBurstState : uint8
    {
        Baseline = 0,
        Active = 1,
        Grace = 2
    };

    struct HubRuntime
    {
        uint32 playerCount{0};
        HubBurstState burst{HubBurstState::Baseline};
        uint32 graceEndMs{0};
    };

    std::unordered_map<std::string, HubRuntime> g_hubs;
    std::unordered_map<uint32, std::string> g_playerHub;
    uint32 g_reconcileAccumMs = 0;

    bool CountsForPresence(Player* player)
    {
        if (!player || !player->IsInWorld())
            return false;
        if (!player->GetSession() || player->GetSession()->IsBot())
            return false;
        if (!CbSettings::GetBool("HybridPopulation"))
            return false;
        return CbSettings::GetBool("CountRealPlayers");
    }

    HubRuntime& Hub(char const* key)
    {
        return g_hubs[std::string(key)];
    }

    void ActivateHub(char const* key)
    {
        HubRuntime& state = Hub(key);
        state.burst = HubBurstState::Active;
        state.graceEndMs = 0;
    }

    void DeactivateHubImmediate(char const* key)
    {
        HubRuntime& state = Hub(key);
        state.burst = HubBurstState::Baseline;
        state.graceEndMs = 0;
    }

    void StartHubGrace(char const* key)
    {
        HubRuntime& state = Hub(key);
        if (state.playerCount > 0)
            return;
        state.burst = HubBurstState::Grace;
        state.graceEndMs = getMSTime() +
            CbSettings::GetUInt("BurstGraceSec") * IN_MILLISECONDS;
    }

    void LeaveHub(uint32 guid, bool useGrace)
    {
        auto it = g_playerHub.find(guid);
        if (it == g_playerHub.end())
            return;

        CityHubDef const* hub = CityHubRegistry::Instance().FindByKey(it->second.c_str());
        g_playerHub.erase(it);
        if (!hub)
            return;

        HubRuntime& state = Hub(hub->key);
        if (state.playerCount > 0)
            --state.playerCount;

        if (state.playerCount == 0)
        {
            if (useGrace)
                StartHubGrace(hub->key);
            else
                DeactivateHubImmediate(hub->key);
        }
    }

    CityHubDef const* FindHubForAreaOrZone(uint32 areaOrZoneId)
    {
        if (!areaOrZoneId)
            return nullptr;

        if (CityHubDef const* hub =
                CityHubRegistry::Instance().FindHubForZone(areaOrZoneId))
            return hub;

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaOrZoneId);
        if (!area || !area->zone || area->zone == areaOrZoneId)
            return nullptr;

        return CityHubRegistry::Instance().FindHubForZone(area->zone);
    }

    CityHubDef const* FindHubForPlayer(Player* player)
    {
        if (!player)
            return nullptr;

        if (CityHubDef const* hub = FindHubForAreaOrZone(player->GetZoneId()))
            return hub;

        return FindHubForAreaOrZone(player->GetAreaId());
    }

    void JoinHub(uint32 guid, CityHubDef const* hub)
    {
        if (!hub)
            return;

        auto it = g_playerHub.find(guid);
        if (it != g_playerHub.end())
        {
            if (it->second == hub->key)
                return;
            LeaveHub(guid, false);
        }

        HubRuntime& state = Hub(hub->key);
        ++state.playerCount;
        g_playerHub[guid] = hub->key;
        ActivateHub(hub->key);
        CbLog::Info("hub burst active: {} (real players {})", hub->key, state.playerCount);
    }

    void ReconcileFromWorld()
    {
        if (!CbSettings::GetBool("HybridPopulation"))
            return;

        std::unordered_map<uint32, std::string> seen;
        for (auto const& kv : ObjectAccessor::GetPlayers())
        {
            Player* player = kv.second;
            if (!CountsForPresence(player))
                continue;

            CityHubDef const* hub = FindHubForPlayer(player);
            if (!hub)
                continue;

            seen[player->GetGUID().GetCounter()] = hub->key;
        }

        std::vector<uint32> missing;
        std::vector<uint32> changed;
        for (auto const& [guid, hubKey] : g_playerHub)
        {
            auto it = seen.find(guid);
            if (it == seen.end())
            {
                missing.push_back(guid);
                continue;
            }

            if (it->second != hubKey)
                changed.push_back(guid);
        }

        for (uint32 guid : missing)
            LeaveHub(guid, true);
        for (uint32 guid : changed)
            LeaveHub(guid, false);

        for (auto const& [guid, hubKey] : seen)
            JoinHub(guid, CityHubRegistry::Instance().FindByKey(hubKey.c_str()));
    }
}

namespace CityPopulationMgr
{
    void RefreshFromWorld()
    {
        g_hubs.clear();
        g_playerHub.clear();
        g_reconcileAccumMs = 0;

        if (!CbSettings::GetBool("HybridPopulation"))
            return;

        ReconcileFromWorld();
    }

    void OnPlayerLogin(Player* player)
    {
        if (!CountsForPresence(player))
            return;

        CityHubDef const* hub = FindHubForPlayer(player);
        if (hub)
            JoinHub(player->GetGUID().GetCounter(), hub);
    }

    void OnPlayerLogout(Player* player)
    {
        if (!player || !CbSettings::GetBool("HybridPopulation"))
            return;

        LeaveHub(player->GetGUID().GetCounter(), true);
    }

    void OnPlayerMapChanged(Player* player)
    {
        if (!CountsForPresence(player))
            return;

        CityHubDef const* hub = FindHubForPlayer(player);
        if (hub)
            JoinHub(player->GetGUID().GetCounter(), hub);
        else
            LeaveHub(player->GetGUID().GetCounter(), true);
    }

    void OnPlayerZoneUpdate(Player* player, uint32 newZone)
    {
        if (!CountsForPresence(player))
            return;

        uint32 guid = player->GetGUID().GetCounter();
        CityHubDef const* oldHub = nullptr;
        auto it = g_playerHub.find(guid);
        if (it != g_playerHub.end())
            oldHub = CityHubRegistry::Instance().FindByKey(it->second.c_str());

        CityHubDef const* newHub = FindHubForAreaOrZone(newZone);

        if (oldHub == newHub)
            return;

        if (oldHub)
            LeaveHub(guid, newHub == nullptr);

        if (newHub)
            JoinHub(guid, newHub);
    }

    void OnPlayerAreaUpdate(Player* player, uint32 newArea)
    {
        if (!CountsForPresence(player))
            return;

        uint32 guid = player->GetGUID().GetCounter();
        CityHubDef const* oldHub = nullptr;
        auto it = g_playerHub.find(guid);
        if (it != g_playerHub.end())
            oldHub = CityHubRegistry::Instance().FindByKey(it->second.c_str());

        CityHubDef const* newHub = FindHubForAreaOrZone(newArea);
        if (!newHub)
            newHub = FindHubForPlayer(player);

        if (oldHub == newHub)
            return;

        if (oldHub)
            LeaveHub(guid, newHub == nullptr);

        if (newHub)
            JoinHub(guid, newHub);
    }

    void Tick(uint32 diff)
    {
        if (!CbSettings::GetBool("HybridPopulation"))
            return;

        g_reconcileAccumMs += diff;
        if (g_reconcileAccumMs >= 5 * IN_MILLISECONDS)
        {
            g_reconcileAccumMs = 0;
            ReconcileFromWorld();
        }

        uint32 now = getMSTime();
        for (auto& [key, state] : g_hubs)
        {
            if (state.burst != HubBurstState::Grace)
                continue;
            if (state.playerCount > 0)
            {
                state.burst = HubBurstState::Active;
                state.graceEndMs = 0;
                continue;
            }
            if (!state.graceEndMs || now < state.graceEndMs)
                continue;
            state.burst = HubBurstState::Baseline;
            state.graceEndMs = 0;
        }
    }

    bool IsHubBurstActive(char const* hubKey)
    {
        if (!hubKey || !CbSettings::GetBool("HybridPopulation"))
            return true;

        auto it = g_hubs.find(std::string(hubKey));
        if (it == g_hubs.end())
            return false;

        HubRuntime const& state = it->second;
        if (state.playerCount > 0)
            return true;
        return state.burst == HubBurstState::Grace;
    }
}
