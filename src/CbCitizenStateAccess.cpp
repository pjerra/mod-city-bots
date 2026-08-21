#include "CbCitizenStateAccess.h"

#include "Ai/City/CityBots/CbValueKeys.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Value.h"

namespace CbCitizenStateAccess
{
    Value<CitizenState>* TryValue(PlayerbotAI* botAI)
    {
        if (!botAI)
            return nullptr;

        return botAI->GetAiObjectContext()->GetValue<CitizenState>(CbKey::CitizenState);
    }

    CitizenState* Try(PlayerbotAI* botAI)
    {
        Value<CitizenState>* value = TryValue(botAI);
        return value ? &value->RefGet() : nullptr;
    }

    bool IsBotReady(Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || !botAI)
            return false;

        if (!bot->GetSession() || !bot->GetSession()->IsBot())
            return false;

        if (!bot->IsInWorld() || !bot->GetMap())
            return false;

        return Try(botAI) != nullptr;
    }

    bool CanMutateWorld(Player* bot)
    {
        return bot && bot->IsInWorld() && bot->GetMap() && !bot->IsBeingTeleported();
    }
}
