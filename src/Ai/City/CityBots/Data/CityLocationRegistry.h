#ifndef MOD_CITYBOTS_CITYLOCATIONREGISTRY_H
#define MOD_CITYBOTS_CITYLOCATIONREGISTRY_H

#include "SharedDefines.h"

#include <string>
#include <vector>

class Player;

struct CityLocationDef
{
    char const* key;
    char const* label;
    uint32 zoneId;
    uint32 mapId;
    TeamId team;
    float anchorX;
    float anchorY;
    float anchorZ;
    float radius;
    bool anyTeam;
    bool duelHub;
    bool socialHub;
    bool fixedPopulation;
};

class CityLocationRegistry
{
public:
    static CityLocationRegistry& Instance();

    std::vector<CityLocationDef> const& All() const { return _locations; }
    CityLocationDef const* FindByKey(char const* key) const;
    CityLocationDef const* FindByKey(std::string const& key) const;

    uint32 GetConfiguredCount(char const* key) const;
    uint32 GetMinCount(char const* key) const;
    uint32 GetDesiredCount(char const* key) const;
    uint32 GetPoolCapacity() const;
    uint32 GetDedicatedTargetCount() const;
    uint32 CountOnlineAtLocation(char const* key) const;
    uint32 CountPresentAtLocation(char const* key) const;
    bool IsBotInLocation(Player* bot, CityLocationDef const& loc) const;
    bool IsBotInHomeLocation(Player* bot, char const* homeKey) const;

    std::string AssignHomeLocation(Player* bot);
    std::string AssignHomeLocationForRace(uint32 guidLow, uint8 race, uint8 level = 0);
    std::string GetStoredHomeLocation(uint32 guidLow) const;
    void StoreHomeLocation(uint32 guidLow, std::string const& key);

    bool IsCitizenAllowedZone(uint32 zoneId) const;
    uint32 GetTotalConfiguredDedicated() const;
    bool UsesDedicatedPool() const;

private:
    CityLocationRegistry();
    std::vector<CityLocationDef> _locations;
};

#endif
