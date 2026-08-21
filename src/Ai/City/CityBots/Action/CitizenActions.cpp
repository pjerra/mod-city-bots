#include "Ai/City/CityBots/Action/CitizenActions.h"

#include "CbCitizenStateAccess.h"
#include "CbCitizenRoles.h"
#include "CbDuelBotUtil.h"
#include "CitizenRosterRegistry.h"
#include "Ai/City/CityBots/CitizenInfo.h"
#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "Ai/City/CityBots/Data/CityPoiRegistry.h"
#include "Ai/City/CityBots/Data/CrowdDialogues.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "CellImpl.h"
#include "DBCStores.h"
#include "EmoteAction.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "NearestGameObjects.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "StringFormat.h"
#include "SocialMgr.h"
#include "WorldSession.h"

#include <algorithm>

namespace
{
    bool NeedsPoiMovement(CitizenActivity activity)
    {
        switch (activity)
        {
            case CITIZEN_WANDER_DISTRICT:
            case CITIZEN_RANDOM_EMOTE:
            case CITIZEN_IDLE:
                return false;
            default:
                return true;
        }
    }

    template<typename Fn>
    void ForEachOnlineRosterMateAtHome(char const* homeKey, Player* skip, Fn&& fn)
    {
        if (!homeKey || !skip)
            return;

        for (CitizenRosterEntry const* entry :
             CitizenRosterRegistry::Instance().ByHome(homeKey))
        {
            if (!entry || entry->guid == skip->GetGUID().GetCounter())
                continue;

            Player* player = sRandomPlayerbotMgr.GetPlayerBot(entry->guid);
            if (!player || !player->IsInWorld())
                continue;
            if (!player->GetSession() || !player->GetSession()->IsBot())
                continue;

            fn(player);
        }
    }

    bool IsGateDuelist(Player* player)
    {
        if (!player)
            return false;

        CitizenRosterEntry const* entry =
            CitizenRosterRegistry::Instance().FindByGuid(player->GetGUID().GetCounter());
        return entry && entry->role == CITIZEN_ROLE_GATE_DUELIST;
    }

    bool CityBotSayAllowed(CitizenState& state)
    {
        if (!CbSettings::GetBool("EnableSay"))
            return false;

        uint32 const cooldownMs = CbSettings::GetUInt("SayCooldownSec") * IN_MILLISECONDS;
        if (GetMSTimeDiffToNow(state.lastSayMs) < cooldownMs)
            return false;

        state.lastSayMs = getMSTime();
        return true;
    }

    void DismountIfMounted(Player* bot)
    {
        if (!bot || !bot->HasMountedAura())
            return;

        bot->Dismount();
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }

    Player* FindNearbyRealPlayer(Player* bot, float radius)
    {
        if (!bot)
            return nullptr;

        Player* best = nullptr;
        float bestDist = radius;
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            (void)guid;
            if (!player || player == bot || !player->IsInWorld())
                continue;
            if (!player->GetSession() || player->GetSession()->IsBot())
                continue;
            if (player->GetMapId() != bot->GetMapId())
                continue;

            float dist = bot->GetDistance(player);
            if (dist <= bestDist)
            {
                best = player;
                bestDist = dist;
            }
        }

        return best;
    }

    // CrowdPhase lives in CitizenInfo.h (shared with the gate direct-drive).

    bool CrowdSceneAwake(Player* bot)
    {
        if (!CbSettings::GetBool("EnableCrowdScenes"))
            return false;
        return FindNearbyRealPlayer(bot,
            static_cast<float>(CbSettings::GetUInt("CrowdPlayerRadius"))) != nullptr;
    }

    using CrowdDialogues::CrowdPartnerPoi;

    void CrowdClearAfk(Player* bot)
    {
        if (bot->isAFK())
            bot->ToggleAFK();
    }

    bool NeedsNpcVisit(CitizenActivity activity)
    {
        switch (activity)
        {
            case CITIZEN_VISIT_AUCTION_HOUSE:
            case CITIZEN_VISIT_MAILBOX:
            case CITIZEN_VISIT_TRAINER:
            case CITIZEN_BUY_FOOD:
                return true;
            default:
                return false;
        }
    }

    bool NeedsSit(CitizenActivity activity)
    {
        switch (activity)
        {
            case CITIZEN_SIT_IN_TAVERN:
            case CITIZEN_DRINK_IN_TAVERN:
            case CITIZEN_WATCH_DUELS:
                return true;
            default:
                return false;
        }
    }

    uint32 NpcFlagForActivity(CitizenActivity activity)
    {
        switch (activity)
        {
            case CITIZEN_VISIT_AUCTION_HOUSE:
                return UNIT_NPC_FLAG_AUCTIONEER;
            case CITIZEN_VISIT_TRAINER:
                return UNIT_NPC_FLAG_TRAINER;
            case CITIZEN_BUY_FOOD:
                return UNIT_NPC_FLAG_VENDOR_FOOD;
            default:
                return 0;
        }
    }

    bool CanDuelHere(Player* player)
    {
        if (!player)
            return false;

        if (sPlayerbotAIConfig.IsInPvpProhibitedZone(player->GetZoneId()))
            return false;

        AreaTableEntry const* area = sAreaTableStore.LookupEntry(player->GetAreaId());
        return !area || (area->flags & AREA_FLAG_ALLOW_DUELS);
    }

    bool CanDuelWith(Player* bot, Player* opponent)
    {
        if (!bot || !opponent || bot == opponent)
            return false;

        if (!opponent->GetSession() || !opponent->GetSession()->IsBot())
            return false;

        if (bot->duel || opponent->duel)
            return false;

        if (bot->IsInCombat() || opponent->IsInCombat())
            return false;

        if (!CanDuelHere(bot) || !CanDuelHere(opponent))
            return false;

        if (bot->GetLevel() != CITIZEN_DUEL_HUB_LEVEL ||
            opponent->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
            return false;

        if (!bot->IsWithinLOSInMap(opponent))
            return false;

        if (opponent->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
            return false;

        if (opponent->GetSocial() && opponent->GetSocial()->HasIgnore(bot->GetGUID()))
            return false;

        return true;
    }

    bool IsDuelHubHome(CitizenState const& state)
    {
        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        return loc && loc->duelHub;
    }

    bool IsGoldshireDuelist(CitizenState const& state)
    {
        return state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;
    }

    bool IsCityDueler(CitizenState const& state)
    {
        return state.role == CITIZEN_ROLE_GATE_DUELIST ||
               state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;
    }

    bool AcceptPendingDuel(Player* bot, CitizenState const& state)
    {
        if (!bot || !IsCityDueler(state) || !bot->duel || !bot->GetSession())
            return false;

        if (bot == bot->duel->Initiator || bot->duel->State != DUEL_STATE_CHALLENGED)
            return false;

        WorldPacket packet(CMSG_DUEL_ACCEPTED, 8);
        packet << bot->GetGuidValue(PLAYER_DUEL_ARBITER);
        bot->GetSession()->HandleDuelAcceptedOpcode(packet);
        return true;
    }

    void HealDuelHubBot(Player* bot)
    {
        if (!bot || bot->duel)
            return;

        if (bot->IsInCombat())
        {
            bot->AttackStop();
            bot->CombatStopWithPets(true);
            bot->ClearInCombat();
        }

        if (!bot->IsAlive() || bot->isDead())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }

        bot->AttackStop();
        bot->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
        bot->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        bot->RemoveAurasByType(SPELL_AURA_FEIGN_DEATH);
        bot->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE);
        bot->RemoveAurasByType(SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
        bot->RemoveAurasByType(SPELL_AURA_PERIODIC_LEECH);
        bot->RemoveAurasByType(SPELL_AURA_MOD_STUN);
        bot->RemoveAurasByType(SPELL_AURA_MOD_FEAR);
        bot->RemoveAurasByType(SPELL_AURA_MOD_CONFUSE);
        bot->RemoveAurasByType(SPELL_AURA_MOD_ROOT);

        if (bot->GetHealthPct() < 95.0f)
            bot->SetFullHealth();

        if (bot->GetPowerPct(POWER_MANA) < 95.0f)
            bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
    }

    bool LeashHomeDuelBot(Player* bot, CitizenState& state)
    {
        if (!IsGoldshireDuelist(state) && !IsDuelHubHome(state))
            return false;

        if (CityLocationRegistry::Instance().IsBotInHomeLocation(
                bot, state.homeLocationKey.c_str()))
            return false;

        CityPoi const* poi = CityPoiRegistry::Instance().PickPoi(
            state.homeLocationKey, CITY_POI_DUEL_RING);
        if (!poi)
            return false;

        WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
        float x = pos.GetPositionX();
        float y = pos.GetPositionY();
        float z = pos.GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);
        bot->TeleportTo(pos.GetMapId(), x, y, z, pos.GetOrientation());
        state.targetPoi = pos;
        state.targetPoiId = poi->id;
        state.atPoi = true;
        state.activity = CITIZEN_DUEL;
        return true;
    }

    bool CanDuelWithGoldshire(Player* bot, Player* opponent)
    {
        if (!bot || !opponent || bot == opponent)
            return false;

        if (!opponent->GetSession() || !opponent->GetSession()->IsBot())
            return false;

        if (bot->duel || opponent->duel)
            return false;

        if (bot->IsInCombat() || opponent->IsInCombat())
            return false;

        if (!CanDuelHere(bot) || !CanDuelHere(opponent))
            return false;

        if (bot->GetLevel() != CITIZEN_DUEL_HUB_LEVEL ||
            opponent->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
            return false;

        if (!bot->IsWithinLOSInMap(opponent))
            return false;

        if (opponent->GetSocial() && opponent->GetSocial()->HasIgnore(bot->GetGUID()))
            return false;

        return true;
    }

    Player* FindGoldshireDuelOpponent(Player* bot, CitizenState const& state)
    {
        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        if (!loc || state.homeLocationKey != "Goldshire")
            return nullptr;

        float searchRadius = std::max<float>(loc->radius, 90.0f);
        Player* best = nullptr;
        float bestDist = searchRadius;

        ForEachOnlineRosterMateAtHome("Goldshire", bot, [&](Player* player)
        {
            CitizenState const* oppState = CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(player));
            if (!oppState || oppState->role != CITIZEN_ROLE_GOLDSHIRE_DUELIST)
                return;

            float distFromAnchor = player->GetDistance(loc->anchorX, loc->anchorY, loc->anchorZ);
            if (distFromAnchor > searchRadius)
                return;

            if (!CanDuelWithGoldshire(bot, player))
                return;

            float dist = bot->GetDistance(player);
            if (dist < bestDist)
            {
                best = player;
                bestDist = dist;
            }
        });

        return best;
    }

    Player* FindGateDuelOpponent(Player* bot, CitizenState const& state)
    {
        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        if (!loc || !loc->duelHub)
            return nullptr;

        float searchRadius = std::max<float>(loc->radius, 90.0f);
        Player* best = nullptr;
        float bestDist = searchRadius;

        ForEachOnlineRosterMateAtHome(state.homeLocationKey.c_str(), bot,
                                      [&](Player* player)
        {
            if (!IsGateDuelist(player))
                return;

            float distFromAnchor = player->GetDistance(loc->anchorX, loc->anchorY, loc->anchorZ);
            if (distFromAnchor > searchRadius)
                return;

            if (!CanDuelWith(bot, player))
                return;

            float dist = bot->GetDistance(player);
            if (dist < bestDist)
            {
                best = player;
                bestDist = dist;
            }
        });

        return best;
    }
}

bool CitizenStatusUpdateAction::Execute(Event /*event*/)
{
    if (!CbCitizenStateAccess::IsBotReady(bot, botAI))
        return false;

    CitizenState& state = State();
    if (AcceptPendingDuel(bot, state))
        return true;

    if (!state.initialized)
    {
        state.initialized = true;
        CbCitizenRoles::ApplyRosterRole(bot, state);

        if (state.homeLocationKey.empty())
        {
            std::string stored = CityLocationRegistry::Instance().GetStoredHomeLocation(
                bot->GetGUID().GetCounter());
            if (!stored.empty())
                state.homeLocationKey = stored;
        }
        if (!state.homeZoneId)
        {
            CityLocationDef const* loc =
                CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
            if (loc)
                state.homeZoneId = loc->zoneId;
            else
                state.homeZoneId = bot->GetZoneId();
        }

        if (!CbCitizenRoles::IsDedicated(state.role))
            PickNextActivity();
        else if (state.role == CITIZEN_ROLE_INN_DANCER && state.targetPoiId &&
                 CbCitizenStateAccess::CanMutateWorld(bot))
        {
            CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId);
            if (poi)
            {
                WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
                bot->TeleportTo(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(),
                                pos.GetPositionZ(), pos.GetOrientation());
                state.targetPoi = pos;
                state.atPoi = true;
                CbCitizenRoles::StripForInnDance(bot);
                state.strippedForDance = true;
            }
        }
        else if ((state.role == CITIZEN_ROLE_GATE_DUELIST ||
                  state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST) &&
                 CbCitizenStateAccess::CanMutateWorld(bot))
        {
            CbDuelBotUtil::EnsureLevel(bot);
        }

        return true;
    }

    if (IsDuelHubHome(state) || IsGoldshireDuelist(state))
    {
        HealDuelHubBot(bot);
        if (LeashHomeDuelBot(bot, state))
            return true;
        return false;
    }

    if (ActivityExpired())
        PickNextActivity();

    return false;
}

bool CitizenMoveToPoiAction::isUseful()
{
    CitizenState const& state = State();
    return NeedsPoiMovement(state.activity) && !state.atPoi;
}

bool CitizenMoveToPoiAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    if (!state.targetPoi.IsValid() && !AssignPoiForActivity(state.activity))
        return false;

    if (IsAtCurrentPoi())
    {
        state.atPoi = true;
        state.mountDecisionPoiId = 0;
        state.mountAttemptMs = 0;
        DismountIfMounted(bot);
        return true;
    }

    return MoveToCurrentPoi();
}

bool CitizenWanderAction::isUseful()
{
    CitizenState const& state = State();
    if (IsDuelHubHome(state))
        return false;

    if (state.activity == CITIZEN_DUEL && (bot->duel || bot->IsInCombat()))
        return false;

    return state.activity == CITIZEN_WANDER_DISTRICT ||
           state.activity == CITIZEN_WALK_CITY_WALLS ||
           state.activity == CITIZEN_DUEL ||
           (state.atPoi && state.activity == CITIZEN_WATCH_DUELS);
}

bool CitizenWanderAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    float step = state.activity == CITIZEN_WALK_CITY_WALLS ? 20.0f : 12.0f;
    if (state.activity == CITIZEN_DUEL)
        step = 18.0f;

    if (state.targetPoi.IsValid())
    {
        float wanderDist = state.activity == CITIZEN_DUEL ? urand(8, 20) : urand(4, 12);
        return MoveNear(state.targetPoi.GetMapId(), state.targetPoi.GetPositionX(),
                        state.targetPoi.GetPositionY(), state.targetPoi.GetPositionZ(), wanderDist);
    }
    return Move(step, frand(0.f, 2.f * static_cast<float>(M_PI)));
}

bool CitizenSitAction::isUseful()
{
    return NeedsSit(State().activity);
}

bool CitizenSitAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    if (!state.atPoi && NeedsPoiMovement(state.activity))
        return false;

    if (!state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_SIT);
        state.sitting = true;

        CityLocationDef const* loc =
            CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        if (loc && loc->socialHub && roll_chance_i(35))
            bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);

        return true;
    }

    if (state.activity == CITIZEN_DRINK_IN_TAVERN && roll_chance_i(20))
    {
        state.lastEmoteMs = getMSTime();
        bot->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
    }

    CityLocationDef const* loc =
        CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
    if (loc && loc->socialHub && state.sitting && roll_chance_i(15))
    {
        state.lastEmoteMs = getMSTime();
        bot->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
    }

    return false;
}

bool CitizenEmoteAction::isUseful()
{
    CitizenState const& state = State();
    if (bot->HasMountedAura() || bot->isMoving() || !state.atPoi ||
        (state.mountAttemptMs && GetMSTimeDiffToNow(state.mountAttemptMs) < 2500))
        return false;

    return state.activity == CITIZEN_RANDOM_EMOTE ||
           (roll_chance_i(CbSettings::GetUInt("RandomSayChance")) &&
            GetMSTimeDiffToNow(state.lastEmoteMs) > 30 * IN_MILLISECONDS);
}

bool CitizenEmoteAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    state.lastEmoteMs = getMSTime();

    CityLocationDef const* loc =
        CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
    if (loc && loc->socialHub && roll_chance_i(40))
    {
        bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
        return true;
    }

    static uint32 const emotes[] = {
        EMOTE_ONESHOT_WAVE,
        EMOTE_ONESHOT_CHEER,
        EMOTE_ONESHOT_LAUGH,
        EMOTE_ONESHOT_BOW,
        EMOTE_ONESHOT_APPLAUD
    };

    bot->HandleEmoteCommand(emotes[urand(0, 4)]);

    if (roll_chance_i(CbSettings::GetUInt("RandomSayChance")) &&
        CityBotSayAllowed(state))
    {
        static char const* phrases[] = {
            "Lovely day in the city.",
            "Anyone seen the auctioneer?",
            "The tavern smells amazing.",
            "Good to be home.",
            "Busy streets today."
        };
        std::string msg = phrases[urand(0, 4)];
        bot->Say(msg, LANG_UNIVERSAL);
    }

    return true;
}

bool CitizenVisitNpcAction::isUseful()
{
    CitizenState const& state = State();
    return NeedsNpcVisit(state.activity) && state.atPoi;
}

bool CitizenVisitNpcAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    uint32 flag = NpcFlagForActivity(state.activity);

    if (state.activity == CITIZEN_VISIT_MAILBOX)
    {
        std::list<GameObject*> targets;
        AnyGameObjectInObjectRangeCheck check(bot, 30.0f);
        Acore::GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(bot, targets, check);
        Cell::VisitObjects(bot, searcher, 30.0f);
        for (GameObject* go : targets)
        {
            if (go && go->GetGoType() == GAMEOBJECT_TYPE_MAILBOX)
            {
                go->Use(bot);
                return true;
            }
        }
        return false;
    }

    std::list<Unit*> targets =
        AI_VALUE(std::list<Unit*>, "possible rpg targets");
    for (Unit* unit : targets)
    {
        if (!unit || !unit->IsCreature())
            continue;
        Creature* creature = bot->GetNPCIfCanInteractWith(unit->GetGUID(), static_cast<NPCFlags>(flag));
        if (!creature)
            continue;
        creature->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->SetFacingToObject(creature);
        WorldPacket packet;
        packet << creature->GetGUID();
        bot->GetSession()->HandleGossipHelloOpcode(packet);
        state.targetNpc = creature->GetGUID();
        return true;
    }

    return false;
}

bool CitizenBuyFoodAction::isUseful()
{
    return State().activity == CITIZEN_BUY_FOOD && State().atPoi;
}

bool CitizenBuyFoodAction::Execute(Event /*event*/)
{
    return botAI->DoSpecificAction("rpg buy", Event("rpg action", "vendor"), true);
}

bool CitizenFishAction::isUseful()
{
    return State().activity == CITIZEN_FISH_AT_DOCKS && CbSettings::GetBool("EnableFishing");
}

bool CitizenFishAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    if (!state.atPoi && !MoveToCurrentPoi())
        return false;

    uint32 fishingSpell = 7620;
    if (!bot->HasSpell(fishingSpell))
        return false;

    if (bot->IsNonMeleeSpellCast(false))
        return false;

    return botAI->CastSpell(fishingSpell, bot);
}

bool CitizenLogoutAction::isUseful()
{
    return State().activity == CITIZEN_LOGOUT && CbSettings::GetBool("EnableLogoutCycle");
}

bool CitizenLogoutAction::Execute(Event /*event*/)
{
    CrowdClearAfk(bot);
    if (WorldSession* session = bot->GetSession())
        session->LogoutPlayer(false);
    return true;
}

bool CitizenDuelAction::isUseful()
{
    CitizenState const& state = State();
    if (state.activity != CITIZEN_DUEL)
        return false;

    // Gate and Goldshire duelers are driven by CbStrategyGate's direct duel
    // state machine. Letting this action also issue challenges creates orphaned
    // duel flags and leaves the area with everyone marked busy.
    if (IsCityDueler(state))
        return false;

    if (bot->duel || bot->IsInCombat())
        return false;

    if (IsGoldshireDuelist(state))
    {
        HealDuelHubBot(bot);
        if (!CityLocationRegistry::Instance().IsBotInHomeLocation(
                bot, state.homeLocationKey.c_str()))
            return false;
        return true;
    }

    if (!IsDuelHubHome(state))
        return false;

    if (bot->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
        return false;

    HealDuelHubBot(bot);

    if (!CityLocationRegistry::Instance().IsBotInHomeLocation(
            bot, state.homeLocationKey.c_str()))
        return false;

    return true;
}

bool CitizenDuelAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    Player* opponent = IsGoldshireDuelist(state)
        ? FindGoldshireDuelOpponent(bot, state)
        : FindGateDuelOpponent(bot, state);

    if (!opponent)
    {
        if (!state.targetPoi.IsValid() && !AssignPoiForActivity(CITIZEN_DUEL))
            return false;

        if (state.targetPoi.IsValid())
            return MoveNear(state.targetPoi.GetMapId(), state.targetPoi.GetPositionX(),
                            state.targetPoi.GetPositionY(), state.targetPoi.GetPositionZ(),
                            IsDuelHubHome(state) ? urand(12, 40) : urand(8, 26));

        return false;
    }

    if (!bot->IsWithinDistInMap(opponent, 10.0f))
        return MoveNear(opponent, IsDuelHubHome(state) ? 8.0f : 4.0f);

    if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, opponent, sPlayerbotAIConfig.sightDistance))
    {
        bot->SetFacingToObject(opponent);
        return true;
    }

    return bot->CastSpell(opponent, 7266, false) == SPELL_CAST_OK;
}

bool CitizenInnDanceAction::isUseful()
{
    CitizenState const& state = State();
    return state.activity == CITIZEN_INN_DANCE && state.atPoi;
}

bool CitizenInnDanceAction::Execute(Event /*event*/)
{
    CitizenState& state = State();

    if (!state.strippedForDance)
    {
        CbCitizenRoles::StripForInnDance(bot);
        state.strippedForDance = true;
    }

    if (state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        state.sitting = false;
    }

    bot->HandleEmoteCommand(EMOTE_STATE_DANCE);

    if (Player* nearby = FindNearbyRealPlayer(bot, 1.0f))
    {
        bot->SetFacingToObject(nearby);
        if (CityBotSayAllowed(state))
        {
            static char const* greetings[] = {
                "Hey there, {}.",
                "Welcome upstairs, {}.",
                "Enjoying Goldshire, {}?",
                "Hi, {}."
            };
            bot->Say(Acore::StringFormat(greetings[urand(0, 3)], nearby->GetName()),
                     LANG_UNIVERSAL);
        }
        return true;
    }

    if (roll_chance_i(20) && CityBotSayAllowed(state))
    {
        static char const* phrases[] = {
            "Welcome to the Lion's Pride!",
            "Best inn in Elwynn!",
            "Another round for the house!",
            "Dance like nobody's watching!"
        };
        bot->Say(phrases[urand(0, 3)], LANG_UNIVERSAL);
    }

    return true;
}

bool CitizenPlazaSocialAction::isUseful()
{
    CitizenState const& state = State();
    return state.activity == CITIZEN_PLAZA_SOCIAL && state.atPoi &&
           !bot->HasMountedAura() && !bot->isMoving();
}

bool CitizenPlazaSocialAction::Execute(Event /*event*/)
{
    CitizenState& state = State();

    if (state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        state.sitting = false;
    }

    Player* nearby = nullptr;
    float bestDist = 18.0f;
    for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
    {
        if (!player || player == bot)
            continue;

        if (!player->GetSession() || !player->GetSession()->IsBot())
            continue;

        if (CityLocationRegistry::Instance().GetStoredHomeLocation(
                player->GetGUID().GetCounter()) != state.homeLocationKey)
            continue;

        float dist = bot->GetDistance(player);
        if (dist < bestDist)
        {
            nearby = player;
            bestDist = dist;
        }
    }

    if (nearby && roll_chance_i(40))
    {
        bot->SetFacingToObject(nearby);
        state.lastEmoteMs = getMSTime();
        if (roll_chance_i(50))
            bot->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
        else
            bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);

        if (CityBotSayAllowed(state))
        {
            static char const* phrases[] = {
                "Hey {}! Lovely day in Goldshire.",
                "Good to see you, {}.",
                "Goldshire's lively today, {}!",
                "Want to grab a drink, {}?"
            };
            std::string line = Acore::StringFormat(
                phrases[urand(0, 3)], nearby->GetName());
            bot->Say(line, LANG_UNIVERSAL);
        }
        return true;
    }

    if (roll_chance_i(35))
    {
        state.lastEmoteMs = getMSTime();
        bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
        return true;
    }
    else if (roll_chance_i(30))
    {
        state.lastEmoteMs = getMSTime();
        bot->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
        return true;
    }
    else if (roll_chance_i(25))
    {
        state.lastEmoteMs = getMSTime();
        bot->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
        return true;
    }

    if (state.targetPoi.IsValid())
        MoveNear(state.targetPoi.GetMapId(), state.targetPoi.GetPositionX(),
                 state.targetPoi.GetPositionY(), state.targetPoi.GetPositionZ(), urand(6, 14));

    if (roll_chance_i(15) && CityBotSayAllowed(state))
    {
        static char const* shouts[] = {
            "Goldshire forever!",
            "Who's up for an adventure?",
            "The forge is busy today.",
            "Beautiful evening in the village."
        };
        bot->Say(shouts[urand(0, 3)], LANG_UNIVERSAL);
    }

    return true;
}

namespace
{
    using CrowdDialogues::CrowdDialogue;
    using CrowdDialogues::kCrowdDialogues;
    using CrowdDialogues::kCrowdDialogueCount;

    // Release a conversation claim on BOTH sides. Safe to call in any state;
    // only clears the partner's side if it still points back at this bot
    // (a self-released partner may already be claimed by a new initiator).
    void CrowdReleaseChat(Player* bot, CitizenState& state)
    {
        if (!state.chatPartner.IsEmpty())
        {
            if (Player* partner = sRandomPlayerbotMgr.GetPlayerBot(state.chatPartner))
                if (CitizenState* partnerState =
                        CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(partner)))
                    if (partnerState->chatPartner == bot->GetGUID())
                    {
                        partnerState->chatPartner.Clear();
                        partnerState->chatInitiator = false;
                        partnerState->chatDialogueId = -1;
                        partnerState->chatLineIdx = 0;
                        partnerState->chatDeadlineMs = 0;
                        partnerState->lastCrowdChatMs = getMSTime();
                    }
        }
        state.chatPartner.Clear();
        state.chatInitiator = false;
        state.chatDialogueId = -1;
        state.chatLineIdx = 0;
        state.chatDeadlineMs = 0;
        state.lastCrowdChatMs = getMSTime();
    }

    Player* CrowdFindChatPartner(Player* bot)
    {
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            (void)guid;
            if (!player || player == bot || !player->IsInWorld())
                continue;
            if (!player->GetSession() || !player->GetSession()->IsBot())
                continue;
            if (player->GetMapId() != bot->GetMapId())
                continue;
            CitizenState* ps = CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(player));
            if (!ps || ps->role != CITIZEN_ROLE_CROWD || ps->crowdPhase != CROWD_LINGER ||
                !ps->chatPartner.IsEmpty() || ps->activity != CITIZEN_CROWD_SOCIAL)
                continue;
            if (bot->GetDistance(player) > 15.0f)
                continue;
            return player;
        }
        return nullptr;
    }

    void CrowdBeginChat(Player* bot, Player* partner, CitizenState& state, uint32 now)
    {
        CitizenState* ps = CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(partner));
        if (!ps)
            return;

        // Pool by the initiator's home POI type; PLAZA pool is the generic fallback.
        CityPoiType pool = CITY_POI_PLAZA;
        if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.crowdHomePoiId))
            if (poi->poiType == CITY_POI_BANK || poi->poiType == CITY_POI_AUCTION_HOUSE)
                pool = poi->poiType;

        std::vector<int16> candidates;
        for (int16 i = 0; i < kCrowdDialogueCount; ++i)
            if (kCrowdDialogues[i].pool == pool || kCrowdDialogues[i].pool == CITY_POI_PLAZA)
                candidates.push_back(i);
        if (candidates.empty())
            return;

        state.chatDialogueId = candidates[urand(0, static_cast<uint32>(candidates.size()) - 1)];
        state.chatPartner = partner->GetGUID();
        state.chatInitiator = true;
        state.chatLineIdx = 0;
        state.chatDeadlineMs = now + 45 * IN_MILLISECONDS;

        ps->chatPartner = bot->GetGUID();
        ps->chatInitiator = false;
        ps->chatDeadlineMs = state.chatDeadlineMs;

        bot->SetFacingToObject(partner);
        partner->SetFacingToObject(bot);
    }
}

bool CitizenCrowdAction::isUseful()
{
    CitizenState const& state = State();
    return state.activity == CITIZEN_CROWD_SOCIAL && state.atPoi &&
           !bot->HasMountedAura() && !bot->isMoving();
}

bool CitizenCrowdAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    uint32 const now = getMSTime();

    if (state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        state.sitting = false;
    }

    // Stale claim, EITHER side: the counterpart vanished or stalled past the
    // deadline, or EnableSay was toggled off mid-exchange. Full release so no
    // claim outlives its deadline (runs even while the scene is asleep).
    if (!state.chatPartner.IsEmpty() && now >= state.chatDeadlineMs)
        CrowdReleaseChat(bot, state);

    if (!state.crowdHomePoiId)
        state.crowdHomePoiId = state.targetPoiId;

    // No real player near: stand cheaply, clear any leftover AFK, do nothing.
    if (!CrowdSceneAwake(bot))
    {
        CrowdClearAfk(bot);
        state.crowdPhase = CROWD_LINGER;
        state.crowdPhaseEndMs = 0;
        return true;
    }

    switch (state.crowdPhase)
    {
        case CROWD_AFK:
            if (now < state.crowdPhaseEndMs)
                return true;                    // stay perfectly still
            CrowdClearAfk(bot);
            state.crowdPhase = CROWD_LINGER;
            state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
            return true;

        case CROWD_WANDER_OUT:
            if (now < state.crowdPhaseEndMs)
                return true;                    // dwelling at the partner POI
            if (CityPoi const* home = CityPoiRegistry::Instance().GetPoi(state.crowdHomePoiId))
            {
                WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*home);
                MoveNear(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(),
                         pos.GetPositionZ(), urand(4, 10));
            }
            state.crowdPhase = CROWD_WANDER_BACK;
            state.crowdPhaseEndMs = now + 30 * IN_MILLISECONDS;
            return true;

        case CROWD_WANDER_BACK:
            if (bot->isMoving() && now < state.crowdPhaseEndMs)
                return true;
            state.crowdPhase = CROWD_LINGER;
            state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
            return true;

        case CROWD_LINGER:
        default:
            break;
    }

    // --- paired chatter (initiator drives both sides) ---
    if (CbSettings::GetBool("EnableSay"))
    {
        // Ongoing conversation I initiated: deliver the next line when due.
        if (state.chatInitiator && state.chatDialogueId >= 0 &&
            state.chatDialogueId < kCrowdDialogueCount)
        {
            if (now >= state.chatDeadlineMs)
                CrowdReleaseChat(bot, state);   // partner vanished or exchange overran
            else
            {
                CrowdDialogue const& d = kCrowdDialogues[state.chatDialogueId];
                if (state.chatLineIdx < d.lineCount &&
                    now >= state.lastSayMs + urand(3000, 6000))
                {
                    bool const partnerLine = (state.chatLineIdx % 2) == 1;
                    Player* speaker = bot;
                    if (partnerLine)
                    {
                        speaker = sRandomPlayerbotMgr.GetPlayerBot(state.chatPartner);
                        if (!speaker || !speaker->IsInWorld() ||
                            bot->GetDistance(speaker) > 20.0f)
                        {
                            CrowdReleaseChat(bot, state);
                            return true;
                        }
                    }
                    speaker->Say(d.lines[state.chatLineIdx], LANG_UNIVERSAL);
                    state.lastSayMs = now;
                    ++state.chatLineIdx;
                    if (state.chatLineIdx >= d.lineCount)
                        CrowdReleaseChat(bot, state);
                }
            }
        }
        // Idle and off cooldown: try to start one.
        else if (state.chatPartner.IsEmpty() &&
                 GetMSTimeDiffToNow(state.lastCrowdChatMs) >
                     CbSettings::GetUInt("CrowdChatCooldownSec") * IN_MILLISECONDS &&
                 roll_chance_i(25))
        {
            if (Player* partner = CrowdFindChatPartner(bot))
                CrowdBeginChat(bot, partner, state, now);
        }
    }

    if (now < state.crowdPhaseEndMs)
    {
        // Occasional idle emote while lingering.
        if (roll_chance_i(10) && GetMSTimeDiffToNow(state.lastEmoteMs) > 25 * IN_MILLISECONDS)
        {
            state.lastEmoteMs = now;
            static uint32 const idleEmotes[] = {
                EMOTE_ONESHOT_TALK, EMOTE_ONESHOT_NO, EMOTE_ONESHOT_POINT,
                EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_QUESTION
            };
            bot->HandleEmoteCommand(idleEmotes[urand(0, 4)]);
        }
        return true;
    }

    // Linger expired, but hold LINGER while a conversation claim is active —
    // leaving LINGER mid-exchange would stall it and strand the partner.
    // Safe: claims are deadline-bounded by the release at the top of Execute.
    if (!state.chatPartner.IsEmpty())
        return true;

    // Linger expired: roll the next phase. 15% AFK, 20% wander, else linger on.
    uint32 const roll = urand(1, 100);
    if (roll <= 15)
    {
        if (!bot->isAFK())
            bot->ToggleAFK();
        state.crowdPhase = CROWD_AFK;
        uint32 afkMinSec = CbSettings::GetUInt("CrowdAfkMinSec");
        uint32 afkMaxSec = CbSettings::GetUInt("CrowdAfkMaxSec");
        if (afkMaxSec < afkMinSec)
            afkMaxSec = afkMinSec;    // same guard as ActivityDurationMs: urand(a, b) needs a <= b
        state.crowdPhaseEndMs = now + urand(afkMinSec, afkMaxSec) * IN_MILLISECONDS;
    }
    else if (roll <= 35)
    {
        if (uint32 partnerPoi = CrowdPartnerPoi(state.crowdHomePoiId))
        {
            if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(partnerPoi))
            {
                WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
                MoveNear(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(),
                         pos.GetPositionZ(), urand(4, 10));
                state.crowdPhase = CROWD_WANDER_OUT;
                state.crowdPhaseEndMs = now + urand(30, 120) * IN_MILLISECONDS;
                return true;
            }
        }
        state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
    }
    else
    {
        state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
    }

    return true;
}
