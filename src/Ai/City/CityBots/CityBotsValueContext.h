#ifndef MOD_CITYBOTS_CITYBOTSVALUECONTEXT_H
#define MOD_CITYBOTS_CITYBOTSVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "Value.h"
#include "Ai/City/CityBots/Value/CitizenStateValue.h"

class CityBotsValueContext : public NamedObjectContext<UntypedValue>
{
public:
    CityBotsValueContext() : NamedObjectContext<UntypedValue>(false, false)
    {
        creators["citizen state"] = &CityBotsValueContext::citizen_state;
    }

private:
    static UntypedValue* citizen_state(PlayerbotAI* ai) { return new CitizenStateValue(ai); }
};

#endif
