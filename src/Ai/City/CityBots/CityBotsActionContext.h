#ifndef MOD_CITYBOTS_CITYBOTSACTIONCONTEXT_H
#define MOD_CITYBOTS_CITYBOTSACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "Action.h"
#include "Ai/City/CityBots/Action/CitizenActions.h"

class CityBotsActionContext : public NamedObjectContext<Action>
{
public:
    CityBotsActionContext() : NamedObjectContext<Action>(false, false)
    {
        creators["citizen status update"] = &CityBotsActionContext::citizen_status_update;
        creators["citizen move to poi"] = &CityBotsActionContext::citizen_move_to_poi;
        creators["citizen wander"] = &CityBotsActionContext::citizen_wander;
        creators["citizen sit"] = &CityBotsActionContext::citizen_sit;
        creators["citizen emote"] = &CityBotsActionContext::citizen_emote;
        creators["citizen visit npc"] = &CityBotsActionContext::citizen_visit_npc;
        creators["citizen buy food"] = &CityBotsActionContext::citizen_buy_food;
        creators["citizen fish"] = &CityBotsActionContext::citizen_fish;
        creators["citizen logout"] = &CityBotsActionContext::citizen_logout;
        creators["citizen duel"] = &CityBotsActionContext::citizen_duel;
        creators["citizen inn dance"] = &CityBotsActionContext::citizen_inn_dance;
        creators["citizen plaza social"] = &CityBotsActionContext::citizen_plaza_social;
        creators["citizen crowd"] = &CityBotsActionContext::citizen_crowd;
    }

private:
    static Action* citizen_status_update(PlayerbotAI* ai) { return new CitizenStatusUpdateAction(ai); }
    static Action* citizen_move_to_poi(PlayerbotAI* ai) { return new CitizenMoveToPoiAction(ai); }
    static Action* citizen_wander(PlayerbotAI* ai) { return new CitizenWanderAction(ai); }
    static Action* citizen_sit(PlayerbotAI* ai) { return new CitizenSitAction(ai); }
    static Action* citizen_emote(PlayerbotAI* ai) { return new CitizenEmoteAction(ai); }
    static Action* citizen_visit_npc(PlayerbotAI* ai) { return new CitizenVisitNpcAction(ai); }
    static Action* citizen_buy_food(PlayerbotAI* ai) { return new CitizenBuyFoodAction(ai); }
    static Action* citizen_fish(PlayerbotAI* ai) { return new CitizenFishAction(ai); }
    static Action* citizen_logout(PlayerbotAI* ai) { return new CitizenLogoutAction(ai); }
    static Action* citizen_duel(PlayerbotAI* ai) { return new CitizenDuelAction(ai); }
    static Action* citizen_inn_dance(PlayerbotAI* ai) { return new CitizenInnDanceAction(ai); }
    static Action* citizen_plaza_social(PlayerbotAI* ai) { return new CitizenPlazaSocialAction(ai); }
    static Action* citizen_crowd(PlayerbotAI* ai) { return new CitizenCrowdAction(ai); }
};

#endif
