#include "Ai/City/CityBots/Action/CitizenBaseAction.h"

#include "CbCitizenRoles.h"
#include "CbCitizenStateAccess.h"
#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "AiObjectContext.h"
#include "Event.h"
#include "Map.h"
#include "Random.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellMgr.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <string>
#include <vector>

namespace
{
    bool IsStormwindTradeDistrict(CityPoi const* poi)
    {
        return poi && poi->locationKey == "Stormwind" &&
               (poi->district == "Trade" || poi->district == "TradeDistrict");
    }

    bool IsOrgrimmarStrengthDistrict(CityPoi const* poi)
    {
        return poi && poi->locationKey == "Orgrimmar" &&
               (poi->district == "Valley" ||
                poi->district == "ValleyOfStrength" ||
                poi->district == "ValleyInn" ||
                poi->district == "ValleyMarket");
    }

    bool DistrictIn(std::string const& district, std::initializer_list<char const*> names)
    {
        for (char const* name : names)
            if (district == name)
                return true;
        return false;
    }

    bool UsesAlternatingHubRoute(std::string const& locationKey)
    {
        return locationKey == "Stormwind" ||
               locationKey == "Orgrimmar" ||
               locationKey == "Ironforge" ||
               locationKey == "Undercity" ||
               locationKey == "Darnassus" ||
               locationKey == "Exodar" ||
               locationKey == "ThunderBluff" ||
               locationKey == "Silvermoon" ||
               locationKey == "Shattrath" ||
               locationKey == "Dalaran";
    }

    std::string HubExcludeDistrict(std::string const& locationKey)
    {
        if (locationKey == "Stormwind")
            return "TradeDistrict";
        if (locationKey == "Orgrimmar")
            return "OrgrimmarHub";
        if (locationKey == "Ironforge")
            return "IronforgeHub";
        if (locationKey == "Undercity")
            return "UndercityHub";
        if (locationKey == "Darnassus")
            return "DarnassusHub";
        if (locationKey == "Exodar")
            return "ExodarHub";
        if (locationKey == "ThunderBluff")
            return "ThunderBluffHub";
        if (locationKey == "Silvermoon")
            return "SilvermoonHub";
        if (locationKey == "Shattrath")
            return "ShattrathHub";
        if (locationKey == "Dalaran")
            return "DalaranHub";
        return "";
    }

    bool IsAlternatingHubPoi(CityPoi const* poi)
    {
        if (!poi)
            return false;

        if (IsStormwindTradeDistrict(poi) || IsOrgrimmarStrengthDistrict(poi))
            return true;

        if (poi->locationKey == "Ironforge")
            return DistrictIn(poi->district, { "GreatForge", "Commons" });
        if (poi->locationKey == "Undercity")
            return DistrictIn(poi->district, { "MagicQuarter" });
        if (poi->locationKey == "Darnassus")
            return DistrictIn(poi->district, { "Craftsmen", "TradesmensTerrace" });
        if (poi->locationKey == "Exodar")
            return DistrictIn(poi->district, { "Seat", "CrystalHall" });
        if (poi->locationKey == "ThunderBluff")
            return DistrictIn(poi->district, { "ElderRise", "LowerRise" });
        if (poi->locationKey == "Silvermoon")
            return DistrictIn(poi->district, { "Sunfury", "Bazaar" });
        if (poi->locationKey == "Shattrath")
            return DistrictIn(poi->district, { "Terrace", "CentralTerrace" });
        if (poi->locationKey == "Dalaran")
            return DistrictIn(poi->district, { "MagusCommerce", "CentralLoop", "PurpleParlor", "Sewers" });

        return false;
    }

    bool IsAlternatingOutsidePoi(CityPoi const* poi)
    {
        if (!poi)
            return false;

        if (poi->locationKey == "Stormwind")
            return !IsStormwindTradeDistrict(poi);

        if (poi->locationKey == "Orgrimmar")
            return poi->district == "Cleft" ||
                   poi->district == "CleftOfShadow" ||
                   poi->district == "Drag" ||
                   poi->district == "DragEntry" ||
                   poi->district == "DurotarGate" ||
                   poi->district == "GateWalk" ||
                   poi->district == "HallOfLegends" ||
                   poi->district == "Honor" ||
                   poi->district == "Ridge" ||
                   poi->district == "TheDrag" ||
                   poi->district == "ValleyOfHonor" ||
                   poi->district == "ValleyOfSpirits" ||
                   poi->district == "ValleyOfWisdom" ||
                   poi->district == "WesternGate";

        if (UsesAlternatingHubRoute(poi->locationKey))
            return !IsAlternatingHubPoi(poi);

        return true;
    }

    bool IsHubPoi(CityPoi const* poi)
    {
        return poi && (IsAlternatingHubPoi(poi) ||
                       poi->poiType == CITY_POI_INN ||
                       poi->poiType == CITY_POI_AUCTION_HOUSE ||
                       poi->poiType == CITY_POI_BANK);
    }

    bool IsHubActivity(CitizenActivity activity)
    {
        return activity == CITIZEN_VISIT_AUCTION_HOUSE ||
               activity == CITIZEN_SIT_IN_TAVERN ||
               activity == CITIZEN_DRINK_IN_TAVERN ||
               activity == CITIZEN_RETURN_TO_INN;
    }

    bool IsGroundMountSpell(SpellInfo const* spellInfo)
    {
        if (!spellInfo || spellInfo->Effects[0].ApplyAuraName != SPELL_AURA_MOUNTED ||
            spellInfo->IsPassive())
            return false;

        if (spellInfo->Effects[1].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            spellInfo->Effects[2].ApplyAuraName == SPELL_AURA_MOD_INCREASE_MOUNTED_FLIGHT_SPEED ||
            spellInfo->Id == 54729)
            return false;

        return true;
    }

    uint32 FindKnownGroundMountSpell(Player* bot)
    {
        if (!bot)
            return 0;

        uint32 bestSpell = 0;
        int32 bestSpeed = -1;
        for (auto const& entry : bot->GetSpellMap())
        {
            uint32 const spellId = entry.first;
            if (!entry.second || entry.second->State == PLAYERSPELL_REMOVED ||
                !entry.second->Active)
                continue;

            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!IsGroundMountSpell(spellInfo))
                continue;

            int32 const speed = std::max(spellInfo->Effects[1].BasePoints,
                                         spellInfo->Effects[2].BasePoints);
            if (speed > bestSpeed)
            {
                bestSpeed = speed;
                bestSpell = spellId;
            }
        }

        return bestSpell;
    }

    void TeachCityTravelMounts(Player* bot)
    {
        if (!bot)
            return;

        static uint32 constexpr ridingSpells[] = { 33388, 33391 };
        for (uint32 spellId : ridingSpells)
            if (!bot->HasSpell(spellId) && sSpellMgr->GetSpellInfo(spellId))
                bot->learnSpell(spellId, false);

        static uint32 constexpr groundMounts[] = {
            470,   // Black Stallion
            580,   // Timber Wolf
            17465, // Green Skeletal Warhorse
            23219, // Swift Mistsaber
            23229, // Swift Brown Steed
            23239, // Swift Gray Ram
            23247, // Great White Kodo
            23250, // Swift Brown Wolf
            35022, // Swift Purple Hawkstrider
            35712  // Great Green Elekk
        };
        for (uint32 spellId : groundMounts)
            if (!bot->HasSpell(spellId) && sSpellMgr->GetSpellInfo(spellId))
                bot->learnSpell(spellId, false);
    }

    bool CastCityTravelMount(PlayerbotAI* botAI, Player* bot)
    {
        if (!botAI || !bot || bot->HasMountedAura() || bot->IsNonMeleeSpellCast(false))
            return false;

        uint32 spellId = FindKnownGroundMountSpell(bot);
        if (!spellId)
        {
            TeachCityTravelMounts(bot);
            spellId = FindKnownGroundMountSpell(bot);
        }

        if (!spellId)
            return false;

        if (bot->isMoving())
            bot->StopMoving();

        if (!bot->IsStandState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);

        return bot->CastSpell(bot, spellId, true) == SPELL_CAST_OK;
    }

    void ClampPoiTravelZ(Player* bot, CityPoi const* poi, WorldPosition& pos)
    {
        if (!bot || !poi || poi->poiType == CITY_POI_INN_BED)
            return;

        float z = pos.GetPositionZ();
        bot->UpdateAllowedPositionZ(pos.GetPositionX(), pos.GetPositionY(), z);

        float groundZ = INVALID_HEIGHT;
        float const walkZ = bot->GetMapWaterOrGroundLevel(
            pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ() + 6.0f,
            &groundZ);
        if (walkZ > INVALID_HEIGHT && std::fabs(z - walkZ) > 4.0f)
            z = walkZ;
        else if (groundZ > INVALID_HEIGHT && std::fabs(z - groundZ) > 4.0f)
            z = groundZ;

        if (bot->GetMap())
        {
            float const mapZ = bot->GetMap()->GetHeight(
                bot->GetPhaseMask(), pos.GetPositionX(), pos.GetPositionY(),
                pos.GetPositionZ() + 6.0f, true, 20.0f);
            if (mapZ > INVALID_HEIGHT && std::fabs(z - mapZ) > 4.0f)
                z = mapZ;
        }

        if (z > INVALID_HEIGHT)
            pos.setZ(z);
    }

    bool MaybeMountForPoiTravel(PlayerbotAI* botAI, Player* bot, CitizenState& state)
    {
        if (!botAI || !bot || bot->duel || bot->IsInCombat() || bot->HasMountedAura() ||
            !bot->IsOutdoors() || !state.targetPoiId || state.mountDecisionPoiId == state.targetPoiId)
            return false;

        if (state.role == CITIZEN_ROLE_INN_DANCER ||
            state.role == CITIZEN_ROLE_GATE_DUELIST ||
            state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST)
            return false;

        if (bot->GetDistance(state.targetPoi.GetPositionX(), state.targetPoi.GetPositionY(),
                             state.targetPoi.GetPositionZ()) < 20.0f)
            return false;

        state.mountDecisionPoiId = state.targetPoiId;
        state.mountAttemptMs = 0;
        uint32 const chance = CbSettings::GetUInt("MountBetweenPoiChance");
        if (chance && roll_chance_i(chance) && CastCityTravelMount(botAI, bot))
        {
            state.mountAttemptMs = getMSTime();
        }

        return false;
    }
}

CitizenState& CitizenBaseAction::State()
{
    CitizenState* state = CbCitizenStateAccess::Try(botAI);
    if (!state)
    {
        static thread_local CitizenState fallback;
        fallback = CitizenState{};
        return fallback;
    }

    return *state;
}

CitizenState const& CitizenBaseAction::State() const
{
    return const_cast<CitizenBaseAction*>(this)->State();
}

CityPoiType CitizenBaseAction::PoiTypeForActivity(CitizenActivity activity) const
{
    switch (activity)
    {
        case CITIZEN_LEAVE_INN:
        case CITIZEN_RETURN_TO_INN:
            return CITY_POI_INN;
        case CITIZEN_VISIT_AUCTION_HOUSE:
            return CITY_POI_AUCTION_HOUSE;
        case CITIZEN_VISIT_MAILBOX:
            return CITY_POI_MAILBOX;
        case CITIZEN_BUY_FOOD:
            return CITY_POI_VENDOR;
        case CITIZEN_SIT_IN_TAVERN:
        case CITIZEN_DRINK_IN_TAVERN:
            return CITY_POI_TAVERN;
        case CITIZEN_VISIT_TRAINER:
            return CITY_POI_TRAINER;
        case CITIZEN_FISH_AT_DOCKS:
            return CITY_POI_DOCK;
        case CITIZEN_WALK_CITY_WALLS:
            return CITY_POI_WALL_PATROL;
        case CITIZEN_WATCH_DUELS:
            return CITY_POI_PLAZA;
        case CITIZEN_DUEL:
            return CITY_POI_DUEL_RING;
        case CITIZEN_INN_DANCE:
            return CITY_POI_INN_BED;
        case CITIZEN_PLAZA_SOCIAL:
            return CITY_POI_PLAZA;
        case CITIZEN_WANDER_DISTRICT:
        case CITIZEN_RANDOM_EMOTE:
            return CITY_POI_PLAZA;
        default:
            return CITY_POI_PLAZA;
    }
}

bool CitizenBaseAction::AssignPoiForActivity(CitizenActivity activity)
{
    CitizenState& state = State();
    if (!state.homeZoneId)
        state.homeZoneId = bot->GetZoneId();

    CityPoiType type = PoiTypeForActivity(activity);
    if (activity == CITIZEN_WATCH_DUELS || activity == CITIZEN_DUEL)
    {
        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        if (loc && (loc->duelHub || state.homeLocationKey == "Goldshire"))
            type = CITY_POI_DUEL_RING;
    }

    CityPoi const* previous = CityPoiRegistry::Instance().GetPoi(state.targetPoiId);
    CityPoi const* poi = nullptr;

    if (UsesAlternatingHubRoute(state.homeLocationKey) && state.role == CITIZEN_ROLE_FLEX)
    {
        bool const previousWasHub = state.stormwindRouteSeen
            ? state.stormwindLastIssuedWasTrade
            : true;
        uint32 const seed = bot->GetGUID().GetCounter() * 2654435761u + getMSTime() / IN_MILLISECONDS;

        if (previousWasHub)
        {
            for (uint32 attempt = 0; attempt < 16; ++attempt)
            {
                poi = CityPoiRegistry::Instance().PickRoamPoi(
                    state.homeLocationKey, seed + attempt * 374761393u, state.targetPoiId, false,
                    HubExcludeDistrict(state.homeLocationKey));
                if (IsAlternatingOutsidePoi(poi))
                    break;

                poi = nullptr;
            }
        }
        else
        {
            CityPoiType const hubTypes[] = {
                CITY_POI_AUCTION_HOUSE,
                CITY_POI_BANK,
                CITY_POI_INN
            };
            uint32 const start = seed % (sizeof(hubTypes) / sizeof(hubTypes[0]));
            for (uint32 i = 0; i < sizeof(hubTypes) / sizeof(hubTypes[0]); ++i)
            {
                poi = CityPoiRegistry::Instance().PickPoi(
                    state.homeLocationKey, hubTypes[(start + i) % (sizeof(hubTypes) / sizeof(hubTypes[0]))],
                    state.targetPoiId);
                if (poi && IsAlternatingHubPoi(poi))
                    break;

                poi = nullptr;
            }
        }

        if (!poi)
            return false;

        state.targetPoiId = poi->id;
        state.targetPoi = CityPoiRegistry::Instance().GetWorldPosition(*poi);
        ClampPoiTravelZ(bot, poi, state.targetPoi);
        state.atPoi = false;
        state.lastDirectRoamWasHub = IsAlternatingHubPoi(poi);
        state.stormwindRouteSeen = true;
        state.stormwindLastIssuedWasTrade = IsAlternatingHubPoi(poi);
        state.mountDecisionPoiId = 0;
        return true;
    }

    bool const previousWasHub = IsHubPoi(previous) || state.lastDirectRoamWasHub;
    std::string const excludeDistrict = previousWasHub && previous ? previous->district : "";

    if (previousWasHub && IsHubActivity(activity))
        type = CITY_POI_PLAZA;

    if (!state.homeLocationKey.empty())
        poi = CityPoiRegistry::Instance().PickPoi(
            state.homeLocationKey, type, state.targetPoiId, excludeDistrict);
    if (!poi && state.homeZoneId)
        poi = CityPoiRegistry::Instance().PickPoi(state.homeZoneId, type, state.targetPoiId);
    if (!poi)
        return false;

    state.targetPoiId = poi->id;
    state.targetPoi = CityPoiRegistry::Instance().GetWorldPosition(*poi);
    ClampPoiTravelZ(bot, poi, state.targetPoi);
    state.atPoi = false;
    state.lastDirectRoamWasHub = IsHubPoi(poi);
    state.mountDecisionPoiId = 0;
    return true;
}

bool CitizenBaseAction::MoveToCurrentPoi(float distance)
{
    CitizenState& state = State();
    if (!state.targetPoiId)
        return false;

    if (!bot->isMoving() && state.lastEmoteMs &&
        GetMSTimeDiffToNow(state.lastEmoteMs) < 8 * IN_MILLISECONDS)
        return false;

    if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId))
        ClampPoiTravelZ(bot, poi, state.targetPoi);

    MaybeMountForPoiTravel(botAI, bot, state);

    return MoveNear(state.targetPoi.GetMapId(), state.targetPoi.GetPositionX(), state.targetPoi.GetPositionY(),
                    state.targetPoi.GetPositionZ(), distance);
}

bool CitizenBaseAction::IsAtCurrentPoi(float distance) const
{
    CitizenState const& state = State();
    if (!state.targetPoiId)
        return false;

    return bot->GetDistance(state.targetPoi.GetPositionX(), state.targetPoi.GetPositionY(),
                          state.targetPoi.GetPositionZ()) <= distance;
}

uint32 CitizenBaseAction::ActivityDurationMs() const
{
    uint32 minSec = CbSettings::GetUInt("ActivityDurationMinSec");
    uint32 maxSec = CbSettings::GetUInt("ActivityDurationMaxSec");
    if (maxSec < minSec)
        maxSec = minSec;
    return (minSec == maxSec ? minSec : urand(minSec, maxSec)) * IN_MILLISECONDS;
}

bool CitizenBaseAction::ActivityExpired() const
{
    CitizenState const& state = State();
    if (CbCitizenRoles::IsDedicated(state.role))
        return false;

    if (!state.activityStartMs)
        return true;
    return GetMSTimeDiffToNow(state.activityStartMs) >= ActivityDurationMs();
}

void CitizenBaseAction::PickNextActivity()
{
    CitizenState& state = State();

    if (CbCitizenRoles::IsDedicated(state.role))
        return;

    struct WeightedActivity
    {
        CitizenActivity activity;
        char const* weightKey;
    };

    static WeightedActivity const pool[] =
    {
        { CITIZEN_VISIT_AUCTION_HOUSE, "Weight.VisitAuctionHouse" },
        { CITIZEN_VISIT_MAILBOX,       "Weight.VisitMailbox"      },
        { CITIZEN_BUY_FOOD,            "Weight.BuyFood"             },
        { CITIZEN_WANDER_DISTRICT,     "Weight.WanderDistrict"      },
        { CITIZEN_SIT_IN_TAVERN,       "Weight.SitInTavern"         },
        { CITIZEN_WATCH_DUELS,         "Weight.WatchDuels"          },
        { CITIZEN_VISIT_TRAINER,       "Weight.VisitTrainer"        },
        { CITIZEN_DRINK_IN_TAVERN,     "Weight.DrinkInTavern"       },
        { CITIZEN_WALK_CITY_WALLS,     "Weight.WalkCityWalls"       },
        { CITIZEN_RETURN_TO_INN,       "Weight.ReturnToInn"         },
        { CITIZEN_LOGOUT,              "Weight.Logout"              },
        { CITIZEN_RANDOM_EMOTE,        "Weight.RandomEmote"         },
        { CITIZEN_FISH_AT_DOCKS,       "Weight.FishAtDocks"         },
    };

    CityLocationDef const* loc =
        CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);

    std::vector<uint32> weighted;

    for (WeightedActivity const& entry : pool)
    {
        if (entry.activity == CITIZEN_FISH_AT_DOCKS && !CbSettings::GetBool("EnableFishing"))
            continue;
        if (entry.activity == CITIZEN_LOGOUT && !CbSettings::GetBool("EnableLogoutCycle"))
            continue;

        uint32 weight = entry.weightKey ? CbSettings::GetUInt(entry.weightKey) : 20;
        if (!weight)
            continue;

        if (state.lastDirectRoamWasHub && IsHubActivity(entry.activity))
            weight = 0;

        if (loc)
        {
            if (loc->socialHub && state.homeLocationKey == "Goldshire")
            {
                if (entry.activity == CITIZEN_VISIT_AUCTION_HOUSE ||
                    entry.activity == CITIZEN_VISIT_MAILBOX ||
                    entry.activity == CITIZEN_VISIT_TRAINER ||
                    entry.activity == CITIZEN_WALK_CITY_WALLS)
                    weight = 0;
            }
            if (loc->duelHub)
            {
                if (entry.activity == CITIZEN_VISIT_AUCTION_HOUSE ||
                    entry.activity == CITIZEN_VISIT_MAILBOX ||
                    entry.activity == CITIZEN_VISIT_TRAINER ||
                    entry.activity == CITIZEN_BUY_FOOD)
                    weight = 0;
            }
        }

        if (!weight)
            continue;

        for (uint32 i = 0; i < weight; ++i)
            weighted.push_back(static_cast<uint32>(entry.activity));
    }

    if (weighted.empty())
        state.activity = CITIZEN_WANDER_DISTRICT;
    else
        state.activity = static_cast<CitizenActivity>(weighted[urand(0, weighted.size() - 1)]);

    if (state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        state.sitting = false;
    }

    state.ResetActivityTimer(getMSTime());
    AssignPoiForActivity(state.activity);
}
