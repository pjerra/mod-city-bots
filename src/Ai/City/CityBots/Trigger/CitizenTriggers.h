#ifndef MOD_CITYBOTS_CITIZENTRIGGERS_H
#define MOD_CITYBOTS_CITIZENTRIGGERS_H

#include "Ai/City/CityBots/CbValueKeys.h"
#include "Ai/City/CityBots/CitizenInfo.h"
#include "Trigger.h"

class CitizenActivityTrigger : public Trigger
{
public:
    CitizenActivityTrigger(PlayerbotAI* botAI, std::string const name, CitizenActivity activity)
        : Trigger(botAI, name), _activity(activity)
    {
    }

    bool IsActive() override;

private:
    CitizenActivity _activity;
};

class CitizenMovePoiTrigger : public Trigger
{
public:
    CitizenMovePoiTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "citizen move poi status")
    {
    }

    bool IsActive() override;
};

class CitizenCombatFleeTrigger : public Trigger
{
public:
    CitizenCombatFleeTrigger(PlayerbotAI* botAI)
        : Trigger(botAI, "citizen combat flee")
    {
    }

    bool IsActive() override;
};

#endif
