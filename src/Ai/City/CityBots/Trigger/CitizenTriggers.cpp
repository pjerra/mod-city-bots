#include "Ai/City/CityBots/Trigger/CitizenTriggers.h"

#include "CbCitizenStateAccess.h"
#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"

bool CitizenActivityTrigger::IsActive()
{
    CitizenState const* state = CbCitizenStateAccess::Try(botAI);
    if (!state)
        return false;

    return state->activity == _activity;
}

bool CitizenMovePoiTrigger::IsActive()
{
    CitizenState const* state = CbCitizenStateAccess::Try(botAI);
    if (!state)
        return false;

    switch (state->activity)
    {
        case CITIZEN_LEAVE_INN:
        case CITIZEN_VISIT_AUCTION_HOUSE:
        case CITIZEN_VISIT_MAILBOX:
        case CITIZEN_BUY_FOOD:
        case CITIZEN_SIT_IN_TAVERN:
        case CITIZEN_WATCH_DUELS:
        case CITIZEN_VISIT_TRAINER:
        case CITIZEN_DRINK_IN_TAVERN:
        case CITIZEN_WALK_CITY_WALLS:
        case CITIZEN_RETURN_TO_INN:
        case CITIZEN_FISH_AT_DOCKS:
        case CITIZEN_INN_DANCE:
        case CITIZEN_PLAZA_SOCIAL:
        case CITIZEN_DUEL:
            return !state->atPoi;
        default:
            return false;
    }
}

bool CitizenCombatFleeTrigger::IsActive()
{
    return bot->IsInCombat();
}
