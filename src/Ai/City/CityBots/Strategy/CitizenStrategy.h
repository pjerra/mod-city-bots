#ifndef MOD_CITYBOTS_CITIZENSTRATEGY_H
#define MOD_CITYBOTS_CITIZENSTRATEGY_H

#include "Strategy.h"

class CitizenStrategy : public Strategy
{
public:
    CitizenStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "citizen"; }
    std::vector<NextAction> getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class CitizenCombatStrategy : public Strategy
{
public:
    CitizenCombatStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "citizen combat"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
