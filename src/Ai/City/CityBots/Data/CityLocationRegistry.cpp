#include "Ai/City/CityBots/Data/CityLocationRegistry.h"

#include "Ai/City/CityBots/CitizenInfo.h"
#include "Ai/City/CityBots/Data/CityHubRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "CityPopulationMgr.h"
#include "CbDuelBotUtil.h"
#include "CitizenRosterRegistry.h"
#include "CbDuelBotUtil.h"

#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    char const* const kHomeEvent = "citizen_home";

    TeamId TeamForRace(uint8 race)
    {
        switch (race)
        {
            case 1: case 3: case 4: case 7: case 11:
                return TEAM_ALLIANCE;
            case 2: case 5: case 6: case 8: case 10:
                return TEAM_HORDE;
            default:
                return TEAM_NEUTRAL;
        }
    }

    bool TeamMatches(TeamId team, CityLocationDef const& loc)
    {
        if (loc.anyTeam)
            return true;
        return team == loc.team;
    }

    bool CanAssignToLocation(CityLocationDef const& loc, TeamId team, uint8 level)
    {
        if (!TeamMatches(team, loc))
            return false;

        return CityLocationRegistry::Instance().GetDesiredCount(loc.key) > 0;
    }

    Player* FindConnectedRosterBot(uint32 guidLow)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
        if (!bot || !bot->IsInWorld() || !bot->GetSession() || !bot->GetSession()->IsBot())
            return nullptr;

        return bot;
    }
}

CityLocationRegistry& CityLocationRegistry::Instance()
{
    static CityLocationRegistry inst;
    return inst;
}

CityLocationRegistry::CityLocationRegistry()
{
    _locations = {
        { "Stormwind",     "Stormwind City",     1519, 0, TEAM_ALLIANCE, 0, 0, 0, 0, false, false, false, false },
        { "Ironforge",     "Ironforge",          1537, 0, TEAM_ALLIANCE, 0, 0, 0, 0, false, false, false, false },
        { "Darnassus",     "Darnassus",          1657, 1, TEAM_ALLIANCE, 0, 0, 0, 0, false, false, false, false },
        { "Exodar",        "The Exodar",         3557, 530, TEAM_ALLIANCE, 0, 0, 0, 0, false, false, false, false },
        { "Orgrimmar",     "Orgrimmar",          1637, 1, TEAM_HORDE, 0, 0, 0, 0, false, false, false, false },
        { "Undercity",     "Undercity",          1497, 0, TEAM_HORDE, 0, 0, 0, 0, false, false, false, false },
        { "ThunderBluff",  "Thunder Bluff",      1638, 1, TEAM_HORDE, 0, 0, 0, 0, false, false, false, false },
        { "Silvermoon",    "Silvermoon City",    3487, 530, TEAM_HORDE, 0, 0, 0, 0, false, false, false, false },
        { "Shattrath",     "Shattrath City",     3703, 530, TEAM_NEUTRAL, 0, 0, 0, 0, true, false, false, true },
        { "Dalaran",       "Dalaran",            4395, 571, TEAM_NEUTRAL, 0, 0, 0, 0, true, false, false, true },
        { "Goldshire",     "Goldshire",          12, 0, TEAM_ALLIANCE,
          -9465.0f, 62.0f, 56.0f, 180.0f, false, false, true, false },
        { "StormwindGate", "Stormwind Gate",     12, 0, TEAM_ALLIANCE,
          -9117.5f, 334.0f, 93.5f, 85.0f, true, true, false, true },
        { "OrgrimmarGate", "Orgrimmar Gate",     14, 1, TEAM_HORDE,
          1280.0f, -4397.5f, 26.35f, 70.0f, true, true, false, true },
    };
}

CityLocationDef const* CityLocationRegistry::FindByKey(char const* key) const
{
    if (!key)
        return nullptr;
    for (CityLocationDef const& loc : _locations)
        if (strcmp(loc.key, key) == 0)
            return &loc;
    return nullptr;
}

CityLocationDef const* CityLocationRegistry::FindByKey(std::string const& key) const
{
    return FindByKey(key.c_str());
}

uint32 CityLocationRegistry::GetConfiguredCount(char const* key) const
{
    char buf[64];
    snprintf(buf, sizeof(buf), "CityCount.%s", key);
    return CbSettings::GetUInt(buf);
}

uint32 CityLocationRegistry::GetMinCount(char const* key) const
{
    char buf[64];
    snprintf(buf, sizeof(buf), "CityCount.Min.%s", key);
    return CbSettings::GetUInt(buf);
}

uint32 CityLocationRegistry::GetDesiredCount(char const* key) const
{
    CityLocationDef const* loc = FindByKey(key);
    if (!loc)
        return 0;

    uint32 maxCount = GetConfiguredCount(key);
    if (!maxCount)
        return 0;

    if (loc->fixedPopulation || !CbSettings::GetBool("HybridPopulation"))
        return maxCount;

    uint32 minCount = GetMinCount(key);
    CityHubDef const* hub = CityHubRegistry::Instance().FindHubForLocation(key);
    if (!hub)
        return maxCount;

    if (CityPopulationMgr::IsHubBurstActive(hub->key))
        return maxCount;

    return std::min(minCount, maxCount);
}

uint32 CityLocationRegistry::GetPoolCapacity() const
{
    uint32 sum = 0;
    for (CityLocationDef const& loc : _locations)
        sum += GetConfiguredCount(loc.key);
    return sum;
}

uint32 CityLocationRegistry::CountOnlineAtLocation(char const* key) const
{
    if (CitizenRosterRegistry::Instance().IsLoaded())
    {
        uint32 count = 0;
        for (CitizenRosterEntry const* entry :
             CitizenRosterRegistry::Instance().ByHome(key))
        {
            Player* bot = FindConnectedRosterBot(entry->guid);
            if (!bot)
                continue;
            if (GetStoredHomeLocation(entry->guid) != key)
                continue;
            ++count;
        }
        return count;
    }

    uint32 count = 0;
    for (auto const& [guid, player] : sRandomPlayerbotMgr.GetAllBots())
    {
        if (!player || !player->IsInWorld())
            continue;
        if (GetStoredHomeLocation(guid.GetCounter()) != key)
            continue;
        ++count;
    }
    return count;
}

uint32 CityLocationRegistry::CountPresentAtLocation(char const* key) const
{
    CityLocationDef const* loc = FindByKey(key);
    if (!loc)
        return 0;

    if (CitizenRosterRegistry::Instance().IsLoaded())
    {
        uint32 count = 0;
        for (CitizenRosterEntry const* entry :
             CitizenRosterRegistry::Instance().ByHome(key))
        {
            Player* bot = FindConnectedRosterBot(entry->guid);
            if (!bot)
                continue;
            if (!IsBotInLocation(bot, *loc))
                continue;
            if (loc->duelHub && !CbDuelBotUtil::IsEligible(bot))
                continue;
            ++count;
        }
        return count;
    }

    uint32 count = 0;
    for (auto const& [guid, player] : sRandomPlayerbotMgr.GetAllBots())
    {
        if (!player || !player->IsInWorld())
            continue;
        if (!IsBotInLocation(player, *loc))
            continue;
        if (loc->duelHub && !CbDuelBotUtil::IsEligible(player))
            continue;
        ++count;
    }
    return count;
}

bool CityLocationRegistry::IsBotInLocation(Player* bot, CityLocationDef const& loc) const
{
    if (!bot)
        return false;

    if (loc.radius <= 0.0f)
    {
        if (bot->GetZoneId() != loc.zoneId)
            return false;
        return true;
    }

    if (bot->GetMapId() != loc.mapId)
        return false;

    return bot->GetDistance(loc.anchorX, loc.anchorY, loc.anchorZ) <= loc.radius;
}

bool CityLocationRegistry::IsBotInHomeLocation(Player* bot, char const* homeKey) const
{
    CityLocationDef const* loc = FindByKey(homeKey);
    if (!loc || !bot)
        return false;
    return IsBotInLocation(bot, *loc);
}

std::string CityLocationRegistry::GetStoredHomeLocation(uint32 guidLow) const
{
    return sRandomPlayerbotMgr.GetData(guidLow, kHomeEvent);
}

void CityLocationRegistry::StoreHomeLocation(uint32 guidLow, std::string const& key)
{
    sRandomPlayerbotMgr.SetValue(guidLow, kHomeEvent, 1, key);
}

std::string CityLocationRegistry::AssignHomeLocationForRace(uint32 guidLow, uint8 race, uint8 level)
{
    std::string existing = GetStoredHomeLocation(guidLow);
    if (!existing.empty())
    {
        CityLocationDef const* existingLoc = FindByKey(existing);
        if (existingLoc && TeamMatches(TeamForRace(race), *existingLoc))
            return existing;
    }

    if (CitizenRosterEntry const* roster = CitizenRosterRegistry::Instance().FindByGuid(guidLow))
    {
        StoreHomeLocation(guidLow, roster->homeKey);
        return roster->homeKey;
    }

    TeamId team = TeamForRace(race);

    std::vector<std::pair<size_t, int32>> deficits;
    for (size_t i = 0; i < _locations.size(); ++i)
    {
        CityLocationDef const& loc = _locations[i];
        if (!CanAssignToLocation(loc, team, level))
            continue;

        uint32 target = GetDesiredCount(loc.key);
        int32 deficit = static_cast<int32>(target) -
                          static_cast<int32>(CountOnlineAtLocation(loc.key));
        if (deficit > 0)
            deficits.emplace_back(i, deficit);
    }

    if (deficits.empty())
    {
        for (CityLocationDef const& loc : _locations)
        {
            if (CanAssignToLocation(loc, team, level))
            {
                StoreHomeLocation(guidLow, loc.key);
                return loc.key;
            }
        }
        std::string fallback = team == TEAM_HORDE ? "Orgrimmar" : "Stormwind";
        StoreHomeLocation(guidLow, fallback);
        return fallback;
    }

    std::vector<size_t> weighted;
    for (auto const& [idx, deficit] : deficits)
    {
        for (int32 i = 0; i < deficit; ++i)
            weighted.push_back(idx);
    }

    size_t pick = weighted[urand(0, weighted.size() - 1)];
    std::string key = _locations[pick].key;
    StoreHomeLocation(guidLow, key);
    return key;
}

std::string CityLocationRegistry::AssignHomeLocation(Player* bot)
{
    std::string key = AssignHomeLocationForRace(
        bot->GetGUID().GetCounter(), bot->getRace(), bot->GetLevel());

    CityLocationDef const* loc = FindByKey(key);
    if (loc && loc->duelHub)
        CbDuelBotUtil::EnsureLevel(bot);

    return key;
}

bool CityLocationRegistry::IsCitizenAllowedZone(uint32 zoneId) const
{
    for (CityLocationDef const& loc : _locations)
        if (loc.zoneId == zoneId && GetDesiredCount(loc.key) > 0)
            return true;
    return false;
}

uint32 CityLocationRegistry::GetTotalConfiguredDedicated() const
{
    return GetPoolCapacity();
}

uint32 CityLocationRegistry::GetDedicatedTargetCount() const
{
    if (!CbSettings::GetBool("HybridPopulation"))
    {
        uint32 citySum = GetPoolCapacity();
        if (citySum)
            return citySum;
        return CbSettings::GetUInt("DedicatedCount");
    }

    uint32 sum = 0;
    for (CityLocationDef const& loc : _locations)
        sum += GetDesiredCount(loc.key);
    return sum;
}

bool CityLocationRegistry::UsesDedicatedPool() const
{
    if (!CbSettings::GetBool("Enable"))
        return false;
    return CitizenRosterRegistry::Instance().IsLoaded() || GetPoolCapacity() > 0;
}
