#ifndef MOD_CITYBOTS_CITIZENSTATEVALUE_H
#define MOD_CITYBOTS_CITIZENSTATEVALUE_H

#include "Ai/City/CityBots/CbValueKeys.h"
#include "Ai/City/CityBots/CitizenInfo.h"
#include "Value.h"

class CitizenStateValue : public ManualSetValue<CitizenState&>
{
public:
    CitizenStateValue(PlayerbotAI* botAI)
        : ManualSetValue<CitizenState&>(botAI, data, CbKey::CitizenState)
    {
    }

    void Reset() override
    {
        data = CitizenState{};
    }

private:
    CitizenState data;
};

#endif
