#ifndef MOD_CITYBOTS_CITIZENBASEACTION_H
#define MOD_CITYBOTS_CITIZENBASEACTION_H

#include "Ai/City/CityBots/CbValueKeys.h"
#include "Ai/City/CityBots/CitizenInfo.h"
#include "Ai/City/CityBots/Data/CityPoiRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Timer.h"

class CitizenBaseAction : public MovementAction
{
public:
    CitizenBaseAction(PlayerbotAI* botAI, std::string const name)
        : MovementAction(botAI, name)
    {
    }

protected:
    CitizenState& State();
    CitizenState const& State() const;

    bool AssignPoiForActivity(CitizenActivity activity);
    CityPoiType PoiTypeForActivity(CitizenActivity activity) const;
    bool MoveToCurrentPoi(float distance = 2.0f);
    bool IsAtCurrentPoi(float distance = 4.0f) const;
    uint32 ActivityDurationMs() const;
    bool ActivityExpired() const;
    void PickNextActivity();
};

#endif
