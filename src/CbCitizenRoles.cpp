#include "CbCitizenRoles.h"

#include "CitizenRosterRegistry.h"
#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "Ai/City/CityBots/Data/CityPoiRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"

#include "Item.h"
#include "Player.h"
#include "SharedDefines.h"

namespace CbCitizenRoles
{
    bool IsDedicated(CitizenRole role)
    {
        return role != CITIZEN_ROLE_FLEX;
    }

    bool IsFemale(Player* bot)
    {
        return bot && bot->getGender() == GENDER_FEMALE;
    }

    void StripForInnDance(Player* bot)
    {
        if (!bot)
            return;

        uint8 const slots[] = {
            EQUIPMENT_SLOT_HEAD,
            EQUIPMENT_SLOT_SHOULDERS,
            EQUIPMENT_SLOT_BODY,
            EQUIPMENT_SLOT_CHEST,
            EQUIPMENT_SLOT_WAIST,
            EQUIPMENT_SLOT_LEGS,
            EQUIPMENT_SLOT_FEET,
            EQUIPMENT_SLOT_WRISTS,
            EQUIPMENT_SLOT_HANDS,
            EQUIPMENT_SLOT_BACK,
            EQUIPMENT_SLOT_MAINHAND,
            EQUIPMENT_SLOT_OFFHAND,
            EQUIPMENT_SLOT_RANGED,
            EQUIPMENT_SLOT_TABARD
        };

        for (uint8 slot : slots)
        {
            if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }
    }

    static CitizenActivity ActivityForRole(CitizenRole role)
    {
        switch (role)
        {
            case CITIZEN_ROLE_INN_DANCER:
                return CITIZEN_INN_DANCE;
            case CITIZEN_ROLE_PLAZA_TALKER:
                return CITIZEN_PLAZA_SOCIAL;
            case CITIZEN_ROLE_GATE_DUELIST:
            case CITIZEN_ROLE_GOLDSHIRE_DUELIST:
                return CITIZEN_DUEL;
            case CITIZEN_ROLE_FISHER:
                return CITIZEN_FISH_AT_DOCKS;
            case CITIZEN_ROLE_CROWD:
                return CITIZEN_CROWD_SOCIAL;
            default:
                return CITIZEN_IDLE;
        }
    }

    void ApplyRosterRole(Player* bot, CitizenState& state)
    {
        if (!bot)
            return;

        CitizenRosterEntry const* entry =
            CitizenRosterRegistry::Instance().FindByGuid(bot->GetGUID().GetCounter());
        if (!entry)
            return;

        state.role = entry->role;
        state.homeLocationKey = entry->homeKey;

        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(entry->homeKey);
        if (loc)
            state.homeZoneId = loc->zoneId;

        if (entry->poiId)
        {
            CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(entry->poiId);
            if (poi && poi->locationKey == entry->homeKey)
            {
                state.targetPoiId = poi->id;
                state.targetPoi = CityPoiRegistry::Instance().GetWorldPosition(*poi);
            }
        }

        if (IsDedicated(entry->role))
        {
            state.activity = ActivityForRole(entry->role);
            state.activityStartMs = getMSTime();
            state.atPoi = false;
            state.sitting = false;
            state.targetNpc.Clear();
        }
    }
}
