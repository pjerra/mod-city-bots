#ifndef MOD_CITYBOTS_CITIZENINFO_H
#define MOD_CITYBOTS_CITIZENINFO_H

#include "Define.h"
#include "ObjectGuid.h"
#include "TravelMgr.h"

enum CitizenRole : uint8
{
    CITIZEN_ROLE_FLEX = 0,
    CITIZEN_ROLE_INN_DANCER = 1,
    CITIZEN_ROLE_PLAZA_TALKER = 2,
    CITIZEN_ROLE_GATE_DUELIST = 3,
    CITIZEN_ROLE_GOLDSHIRE_DUELIST = 4,
    CITIZEN_ROLE_FISHER = 5,
    CITIZEN_ROLE_CROWD = 6
};

enum CitizenActivity : uint8
{
    CITIZEN_LEAVE_INN = 0,
    CITIZEN_VISIT_AUCTION_HOUSE = 1,
    CITIZEN_VISIT_MAILBOX = 2,
    CITIZEN_BUY_FOOD = 3,
    CITIZEN_WANDER_DISTRICT = 4,
    CITIZEN_SIT_IN_TAVERN = 5,
    CITIZEN_WATCH_DUELS = 6,
    CITIZEN_VISIT_TRAINER = 7,
    CITIZEN_DRINK_IN_TAVERN = 8,
    CITIZEN_WALK_CITY_WALLS = 9,
    CITIZEN_RETURN_TO_INN = 10,
    CITIZEN_LOGOUT = 11,
    CITIZEN_RANDOM_EMOTE = 12,
    CITIZEN_FISH_AT_DOCKS = 13,
    CITIZEN_IDLE = 14,
    CITIZEN_DUEL = 15,
    CITIZEN_INN_DANCE = 16,
    CITIZEN_PLAZA_SOCIAL = 17,
    CITIZEN_CROWD_SOCIAL = 18
};

enum CityPoiType : uint8
{
    CITY_POI_INN = 0,
    CITY_POI_AUCTION_HOUSE = 1,
    CITY_POI_MAILBOX = 2,
    CITY_POI_TAVERN = 3,
    CITY_POI_TRAINER = 4,
    CITY_POI_DOCK = 5,
    CITY_POI_WALL_PATROL = 6,
    CITY_POI_PLAZA = 7,
    CITY_POI_VENDOR = 8,
    CITY_POI_DUEL_RING = 9,
    CITY_POI_INN_BED = 10,
    CITY_POI_BANK = 11
};

// Crowd scene phases (living-crowd design). Shared by the action-layer
// CitizenCrowdAction and the CbStrategyGate direct-drive port — keep in sync
// with the crowdPhase comment below.
enum CrowdPhase : uint8
{
    CROWD_LINGER = 0,
    CROWD_AFK = 1,
    CROWD_WANDER_OUT = 2,
    CROWD_WANDER_BACK = 3
};

struct CitizenState
{
    CitizenRole role{CITIZEN_ROLE_FLEX};
    CitizenActivity activity{CITIZEN_IDLE};
    uint32 activityStartMs{0};
    uint32 homeZoneId{0};
    std::string homeLocationKey;
    uint32 targetPoiId{0};
    WorldPosition targetPoi{};
    ObjectGuid targetNpc{};
    bool initialized{false};
    bool atPoi{false};
    bool sitting{false};
    uint32 lastEmoteMs{0};
    uint32 lastSayMs{0};
    bool lastDirectRoamWasHub{false};
    bool stormwindRouteSeen{false};
    bool stormwindLastIssuedWasTrade{true};
    bool strippedForDance{false};
    uint32 mountDecisionPoiId{0};
    uint32 mountAttemptMs{0};

    // living-crowd scene state (see docs/specs/2026-08-11-living-crowd-design.md)
    uint8 crowdPhase{0};            // 0 linger, 1 afk, 2 wander-out, 3 wander-back
    uint32 crowdPhaseEndMs{0};      // when the current phase expires
    uint32 crowdHomePoiId{0};       // POI to return to after WANDER
    uint32 lastCrowdChatMs{0};      // per-citizen conversation cooldown stamp
    ObjectGuid chatPartner{};       // set on BOTH sides while claimed
    uint32 chatDeadlineMs{0};       // claim auto-release deadline
    uint8 chatLineIdx{0};           // next line to deliver (initiator only)
    int16 chatDialogueId{-1};       // index into the dialogue table (initiator only)
    bool chatInitiator{false};
    // One-shot login staging (see CbStrategyGate Reconcile): fresh state means
    // a fresh login; the citizen gets teleport-placed through the fixed
    // resolver exactly once, killing positions saved under the mesh.
    bool loginStaged{false};

    void ResetActivityTimer(uint32 now)
    {
        activityStartMs = now;
        atPoi = false;
        sitting = false;
        strippedForDance = false;
        targetNpc.Clear();
    }
};

// Dedicated stage-cast type. Do not use 2 (playerbots AddClass pool).
constexpr uint8 CITIZEN_ACCOUNT_TYPE = 3;
constexpr uint8 CITIZEN_DUEL_HUB_LEVEL = 80;

// Fixed stage cast (see data/sql/*/updates/2026_07_15_*)
constexpr uint32 STAGE_CAST_CAPACITY = 400;
constexpr uint32 STAGE_CAST_GUID_START = 9'000'001;
constexpr uint32 STAGE_CAST_GUID_END = 9'000'401;
constexpr uint32 STAGE_CAST_ACCOUNT_START = 12001;
constexpr uint32 STAGE_CAST_ACCOUNT_END = 12401;
constexpr uint32 STAGE_CAST_ACCOUNT_COUNT = 400;

#endif
