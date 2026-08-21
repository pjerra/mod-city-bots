#include "Ai/City/CityBots/Data/CityHubRegistry.h"

#include <cstring>

namespace
{
    uint32 const kStormwindZones[] = { 12, 1519 };
    char const* const kStormwindLocs[] = { "Stormwind", "Goldshire", "StormwindGate" };

    uint32 const kIronforgeZones[] = { 1, 1537 };
    char const* const kIronforgeLocs[] = { "Ironforge" };

    uint32 const kDarnassusZones[] = { 141, 1657 };
    char const* const kDarnassusLocs[] = { "Darnassus" };

    uint32 const kExodarZones[] = { 3524, 3557 };
    char const* const kExodarLocs[] = { "Exodar" };

    uint32 const kOrgrimmarZones[] = { 14, 1637 };
    char const* const kOrgrimmarLocs[] = { "Orgrimmar", "OrgrimmarGate" };

    uint32 const kUndercityZones[] = { 85, 1497 };
    char const* const kUndercityLocs[] = { "Undercity" };

    uint32 const kThunderBluffZones[] = { 215, 1638 };
    char const* const kThunderBluffLocs[] = { "ThunderBluff" };

    uint32 const kSilvermoonZones[] = { 3430, 3433, 3487 };
    char const* const kSilvermoonLocs[] = { "Silvermoon" };
}

CityHubRegistry& CityHubRegistry::Instance()
{
    static CityHubRegistry inst;
    return inst;
}

CityHubRegistry::CityHubRegistry()
{
    _hubs = {
        { "StormwindHub", kStormwindZones, 2, kStormwindLocs, 3 },
        { "IronforgeHub", kIronforgeZones, 2, kIronforgeLocs, 1 },
        { "DarnassusHub", kDarnassusZones, 2, kDarnassusLocs, 1 },
        { "ExodarHub", kExodarZones, 2, kExodarLocs, 1 },
        { "OrgrimmarHub", kOrgrimmarZones, 2, kOrgrimmarLocs, 2 },
        { "UndercityHub", kUndercityZones, 2, kUndercityLocs, 1 },
        { "ThunderBluffHub", kThunderBluffZones, 2, kThunderBluffLocs, 1 },
        { "SilvermoonHub", kSilvermoonZones, 3, kSilvermoonLocs, 1 },
    };
}

CityHubDef const* CityHubRegistry::FindByKey(char const* key) const
{
    if (!key)
        return nullptr;
    for (CityHubDef const& hub : _hubs)
        if (strcmp(hub.key, key) == 0)
            return &hub;
    return nullptr;
}

CityHubDef const* CityHubRegistry::FindHubForZone(uint32 zoneId) const
{
    for (CityHubDef const& hub : _hubs)
    {
        for (std::size_t i = 0; i < hub.zoneCount; ++i)
            if (hub.zoneIds[i] == zoneId)
                return &hub;
    }
    return nullptr;
}

CityHubDef const* CityHubRegistry::FindHubForLocation(char const* locationKey) const
{
    if (!locationKey)
        return nullptr;

    for (CityHubDef const& hub : _hubs)
    {
        for (std::size_t i = 0; i < hub.locationCount; ++i)
            if (strcmp(hub.locationKeys[i], locationKey) == 0)
                return &hub;
    }
    return nullptr;
}
