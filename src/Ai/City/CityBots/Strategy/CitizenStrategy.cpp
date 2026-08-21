#include "Ai/City/CityBots/Strategy/CitizenStrategy.h"

CitizenStrategy::CitizenStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

std::vector<NextAction> CitizenStrategy::getDefaultActions()
{
    return { NextAction("citizen status update", 11.0f) };
}

void CitizenStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("citizen move poi status",
        { NextAction("citizen move to poi", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen wander district status",
        { NextAction("citizen wander", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen walk walls status",
        { NextAction("citizen wander", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen duel status",
        { NextAction("citizen duel", 8.0f) }));

    triggers.push_back(new TriggerNode("citizen watch duels status",
        { NextAction("citizen wander", 2.5f), NextAction("citizen sit", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen sit tavern status",
        { NextAction("citizen sit", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen drink tavern status",
        { NextAction("citizen sit", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen visit ah status",
        { NextAction("citizen visit npc", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen visit mailbox status",
        { NextAction("citizen visit npc", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen visit trainer status",
        { NextAction("citizen visit npc", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen buy food status",
        { NextAction("citizen buy food", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen random emote status",
        { NextAction("citizen emote", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen inn dance status",
        { NextAction("citizen move to poi", 4.0f), NextAction("citizen inn dance", 6.0f) }));

    triggers.push_back(new TriggerNode("citizen plaza social status",
        { NextAction("citizen move to poi", 3.0f), NextAction("citizen plaza social", 6.0f) }));

    triggers.push_back(new TriggerNode("citizen crowd status",
        { NextAction("citizen move to poi", 3.0f), NextAction("citizen crowd", 6.0f) }));

    triggers.push_back(new TriggerNode("citizen fish docks status",
        { NextAction("citizen fish", 3.0f) }));

    triggers.push_back(new TriggerNode("citizen logout status",
        { NextAction("citizen logout", 3.0f) }));
}

CitizenCombatStrategy::CitizenCombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

void CitizenCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("citizen combat flee",
        { NextAction("flee", 10.0f) }));
}
