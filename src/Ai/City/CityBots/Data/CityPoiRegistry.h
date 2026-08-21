#ifndef MOD_CITYBOTS_CITYPOIREGISTRY_H
#define MOD_CITYBOTS_CITYPOIREGISTRY_H

#include "Define.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "Ai/City/CityBots/CitizenInfo.h"
#include "TravelMgr.h"

struct CityPoi
{
    uint32 id{0};
    std::string locationKey;
    uint32 zoneId{0};
    CityPoiType poiType{CITY_POI_INN};
    uint32 mapId{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float orientation{0.0f};
    std::string district;
    uint32 weight{100};
};

class CityPoiRegistry
{
public:
    static CityPoiRegistry& Instance();

    void LoadFromDatabase();
    bool IsCitizenZone(uint32 zoneId) const;

    CityPoi const* PickPoi(std::string const& locationKey, CityPoiType type,
                            uint32 excludePoiId = 0,
                            std::string const& excludeDistrict = "") const;
    CityPoi const* PickPoi(uint32 zoneId, CityPoiType type, uint32 excludePoiId = 0) const;
    CityPoi const* PickRoamPoi(std::string const& locationKey, uint32 seed,
                               uint32 excludePoiId = 0, bool includeHubs = true,
                               std::string const& excludeDistrict = "") const;
    CityPoi const* GetPoi(uint32 poiId) const;
    WorldPosition GetWorldPosition(CityPoi const& poi) const;

    // Lowest curated z among the home POIs within radius (2D) of x,y; returns
    // false if none is in range. A citizen more than `margin` below THIS is
    // under every floor the city has at that spot -- a real sink -- whereas
    // "below the nearest POI" also fires on the lower tier of a multi-level
    // city (Undercity -62 vs -43, Ironforge, Dalaran: 243 healthy bots were
    // teleport-yanked per boot by that version).
    bool LowestPoiZWithin(std::string const& locationKey, float x, float y,
                          float radius, float* outMinZ) const;

    std::vector<CityPoi> const& All() const { return _pois; }

private:
    CityPoiRegistry() = default;
    void SeedFallbackData();

    std::vector<CityPoi> _pois;
    std::vector<uint32> _capitalZoneIds;
    std::unordered_map<uint32, uint32> _occupancy;
};

#endif
