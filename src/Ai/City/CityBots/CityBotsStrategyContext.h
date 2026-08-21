#ifndef MOD_CITYBOTS_CITYBOTSSTRATEGYCONTEXT_H
#define MOD_CITYBOTS_CITYBOTSSTRATEGYCONTEXT_H

#include "NamedObjectContext.h"
#include "Strategy.h"
#include "Ai/City/CityBots/Strategy/CitizenStrategy.h"

class CityBotsStrategyContext : public NamedObjectContext<Strategy>
{
public:
    CityBotsStrategyContext() : NamedObjectContext<Strategy>(false, false)
    {
        creators["citizen"] = &CityBotsStrategyContext::citizen;
        creators["citizen combat"] = &CityBotsStrategyContext::citizen_combat;
    }

private:
    static Strategy* citizen(PlayerbotAI* ai) { return new CitizenStrategy(ai); }
    static Strategy* citizen_combat(PlayerbotAI* ai) { return new CitizenCombatStrategy(ai); }
};

#endif
