#ifndef MOD_CITYBOTS_CITYBOTSTRIGGERCONTEXT_H
#define MOD_CITYBOTS_CITYBOTSTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Ai/City/CityBots/Trigger/CitizenTriggers.h"

class CityBotsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CityBotsTriggerContext() : NamedObjectContext<Trigger>(false, false)
    {
        creators["citizen move poi status"] = &CityBotsTriggerContext::move_poi;
        creators["citizen wander district status"] = &CityBotsTriggerContext::wander_district;
        creators["citizen walk walls status"] = &CityBotsTriggerContext::walk_walls;
        creators["citizen watch duels status"] = &CityBotsTriggerContext::watch_duels;
        creators["citizen duel status"] = &CityBotsTriggerContext::duel;
        creators["citizen sit tavern status"] = &CityBotsTriggerContext::sit_tavern;
        creators["citizen drink tavern status"] = &CityBotsTriggerContext::drink_tavern;
        creators["citizen visit ah status"] = &CityBotsTriggerContext::visit_ah;
        creators["citizen visit mailbox status"] = &CityBotsTriggerContext::visit_mailbox;
        creators["citizen visit trainer status"] = &CityBotsTriggerContext::visit_trainer;
        creators["citizen buy food status"] = &CityBotsTriggerContext::buy_food;
        creators["citizen random emote status"] = &CityBotsTriggerContext::random_emote;
        creators["citizen fish docks status"] = &CityBotsTriggerContext::fish_docks;
        creators["citizen logout status"] = &CityBotsTriggerContext::logout;
        creators["citizen inn dance status"] = &CityBotsTriggerContext::inn_dance;
        creators["citizen plaza social status"] = &CityBotsTriggerContext::plaza_social;
        creators["citizen crowd status"] = &CityBotsTriggerContext::crowd;
        creators["citizen combat flee"] = &CityBotsTriggerContext::combat_flee;
    }

private:
    static Trigger* move_poi(PlayerbotAI* ai) { return new CitizenMovePoiTrigger(ai); }
    static Trigger* wander_district(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen wander district status", CITIZEN_WANDER_DISTRICT);
    }
    static Trigger* walk_walls(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen walk walls status", CITIZEN_WALK_CITY_WALLS);
    }
    static Trigger* watch_duels(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen watch duels status", CITIZEN_WATCH_DUELS);
    }
    static Trigger* duel(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen duel status", CITIZEN_DUEL);
    }
    static Trigger* sit_tavern(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen sit tavern status", CITIZEN_SIT_IN_TAVERN);
    }
    static Trigger* drink_tavern(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen drink tavern status", CITIZEN_DRINK_IN_TAVERN);
    }
    static Trigger* visit_ah(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen visit ah status", CITIZEN_VISIT_AUCTION_HOUSE);
    }
    static Trigger* visit_mailbox(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen visit mailbox status", CITIZEN_VISIT_MAILBOX);
    }
    static Trigger* visit_trainer(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen visit trainer status", CITIZEN_VISIT_TRAINER);
    }
    static Trigger* buy_food(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen buy food status", CITIZEN_BUY_FOOD);
    }
    static Trigger* random_emote(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen random emote status", CITIZEN_RANDOM_EMOTE);
    }
    static Trigger* fish_docks(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen fish docks status", CITIZEN_FISH_AT_DOCKS);
    }
    static Trigger* logout(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen logout status", CITIZEN_LOGOUT);
    }
    static Trigger* inn_dance(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen inn dance status", CITIZEN_INN_DANCE);
    }
    static Trigger* plaza_social(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen plaza social status", CITIZEN_PLAZA_SOCIAL);
    }
    static Trigger* crowd(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen crowd status", CITIZEN_CROWD_SOCIAL);
    }
    static Trigger* combat_flee(PlayerbotAI* ai) { return new CitizenCombatFleeTrigger(ai); }
};

#endif
