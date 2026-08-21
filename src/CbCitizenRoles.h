#ifndef MOD_CITYBOTS_CBCITIZENROLES_H
#define MOD_CITYBOTS_CBCITIZENROLES_H

#include "Ai/City/CityBots/CitizenInfo.h"

class Player;

namespace CbCitizenRoles
{
    bool IsDedicated(CitizenRole role);
    void ApplyRosterRole(Player* bot, CitizenState& state);
    void StripForInnDance(Player* bot);
    bool IsFemale(Player* bot);
}

#endif
