#ifndef MOD_CITYBOTS_CITYHUBREGISTRY_H
#define MOD_CITYBOTS_CITYHUBREGISTRY_H

#include "Define.h"

#include <vector>

struct CityHubDef
{
    char const* key;
    uint32 const* zoneIds;
    std::size_t zoneCount;
    char const* const* locationKeys;
    std::size_t locationCount;
};

class CityHubRegistry
{
public:
    static CityHubRegistry& Instance();

    std::vector<CityHubDef> const& All() const { return _hubs; }
    CityHubDef const* FindByKey(char const* key) const;
    CityHubDef const* FindHubForZone(uint32 zoneId) const;
    CityHubDef const* FindHubForLocation(char const* locationKey) const;

private:
    CityHubRegistry();
    std::vector<CityHubDef> _hubs;
};

#endif
