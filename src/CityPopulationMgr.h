#ifndef MOD_CITYBOTS_CITYPOPULATIONMGR_H
#define MOD_CITYBOTS_CITYPOPULATIONMGR_H

#include "Define.h"

class Player;

namespace CityPopulationMgr
{
    void RefreshFromWorld();
    void OnPlayerLogin(Player* player);
    void OnPlayerLogout(Player* player);
    void OnPlayerMapChanged(Player* player);
    void OnPlayerZoneUpdate(Player* player, uint32 newZone);
    void OnPlayerAreaUpdate(Player* player, uint32 newArea);
    void Tick(uint32 diff);

    bool IsHubBurstActive(char const* hubKey);
}

#endif
