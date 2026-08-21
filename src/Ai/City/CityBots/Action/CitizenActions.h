#ifndef MOD_CITYBOTS_CITIZENACTIONS_H
#define MOD_CITYBOTS_CITIZENACTIONS_H

#include "Ai/City/CityBots/Action/CitizenBaseAction.h"

class CitizenStatusUpdateAction : public CitizenBaseAction
{
public:
    CitizenStatusUpdateAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen status update")
    {
    }

    bool Execute(Event event) override;
};

class CitizenMoveToPoiAction : public CitizenBaseAction
{
public:
    CitizenMoveToPoiAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen move to poi")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenWanderAction : public CitizenBaseAction
{
public:
    CitizenWanderAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen wander")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenSitAction : public CitizenBaseAction
{
public:
    CitizenSitAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen sit")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenEmoteAction : public CitizenBaseAction
{
public:
    CitizenEmoteAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen emote")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenVisitNpcAction : public CitizenBaseAction
{
public:
    CitizenVisitNpcAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen visit npc")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenBuyFoodAction : public CitizenBaseAction
{
public:
    CitizenBuyFoodAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen buy food")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenFishAction : public CitizenBaseAction
{
public:
    CitizenFishAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen fish")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenLogoutAction : public CitizenBaseAction
{
public:
    CitizenLogoutAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen logout")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenDuelAction : public CitizenBaseAction
{
public:
    CitizenDuelAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen duel")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenInnDanceAction : public CitizenBaseAction
{
public:
    CitizenInnDanceAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen inn dance")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenPlazaSocialAction : public CitizenBaseAction
{
public:
    CitizenPlazaSocialAction(PlayerbotAI* botAI)
        : CitizenBaseAction(botAI, "citizen plaza social")
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class CitizenCrowdAction : public CitizenBaseAction
{
public:
    CitizenCrowdAction(PlayerbotAI* botAI) : CitizenBaseAction(botAI, "citizen crowd") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
