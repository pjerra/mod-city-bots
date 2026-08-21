#ifndef MOD_CITYBOTS_CBCITIZENSTATEACCESS_H
#define MOD_CITYBOTS_CBCITIZENSTATEACCESS_H

#include "Ai/City/CityBots/CitizenInfo.h"

class Player;
class PlayerbotAI;

template <class T>
class Value;

namespace CbCitizenStateAccess
{
    Value<CitizenState>* TryValue(PlayerbotAI* botAI);
    CitizenState* Try(PlayerbotAI* botAI);
    bool IsBotReady(Player* bot, PlayerbotAI* botAI);
    bool CanMutateWorld(Player* bot);
}

#endif
