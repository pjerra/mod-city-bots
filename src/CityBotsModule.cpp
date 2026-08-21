/*
 * mod-city-bots — CityBotsModule.cpp
 */

#ifdef MOD_PLAYERBOTS

#include "ScriptMgr.h"
#include "Log.h"
#include "Player.h"
#include "PlayerScript.h"

#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"

#include "AiObjectContextAccess.h"
#include "CbStrategyGate.h"
#include "CbCitizenAccountMgr.h"
#include "CbCitizenLoginMgr.h"
#include "CbLog.h"
#include "CityBotsRuntime.h"
#include "CityPopulationMgr.h"
#include "BotActivityRegistry.h"
#include "CitizenRosterRegistry.h"

#include "DKAiObjectContext.h"
#include "DruidAiObjectContext.h"
#include "HunterAiObjectContext.h"
#include "MageAiObjectContext.h"
#include "PaladinAiObjectContext.h"
#include "PriestAiObjectContext.h"
#include "RogueAiObjectContext.h"
#include "ShamanAiObjectContext.h"
#include "WarlockAiObjectContext.h"
#include "WarriorAiObjectContext.h"

#include "Ai/City/CityBots/CityBotsActionContext.h"
#include "Ai/City/CityBots/CityBotsStrategyContext.h"
#include "Ai/City/CityBots/CityBotsTriggerContext.h"
#include "Ai/City/CityBots/CityBotsValueContext.h"
#include "Ai/City/CityBots/Data/CityPoiRegistry.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"

namespace
{
    constexpr uint32 CB_STRATEGY_GATE_SWEEP_MS = 3 * 1000;

    template <class Ctx>
    void RegisterClassContexts()
    {
        Ctx::sharedStrategyContexts.Add(new CityBotsStrategyContext());
        Ctx::sharedActionContexts.Add(new CityBotsActionContext());
        Ctx::sharedTriggerContexts.Add(new CityBotsTriggerContext());
        Ctx::sharedValueContexts.Add(new CityBotsValueContext());
    }
}

class CityBotsRegistrarWorldScript : public WorldScript
{
public:
    CityBotsRegistrarWorldScript() : WorldScript("CityBotsRegistrarWorldScript") {}

    void OnUpdate(uint32 /*diff*/) override
    {
        if (_registered)
            return;
        _registered = true;

        CityPoiRegistry::Instance().LoadFromDatabase();

        RegisterClassContexts<WarriorAiObjectContext>();
        RegisterClassContexts<PaladinAiObjectContext>();
        RegisterClassContexts<DruidAiObjectContext>();
        RegisterClassContexts<DKAiObjectContext>();
        RegisterClassContexts<HunterAiObjectContext>();
        RegisterClassContexts<MageAiObjectContext>();
        RegisterClassContexts<PriestAiObjectContext>();
        RegisterClassContexts<RogueAiObjectContext>();
        RegisterClassContexts<ShamanAiObjectContext>();
        RegisterClassContexts<WarlockAiObjectContext>();

        cb_access::SharedStrategyContexts()->Add(new CityBotsStrategyContext());
        cb_access::SharedActionContexts()->Add(new CityBotsActionContext());
        cb_access::SharedTriggerContexts()->Add(new CityBotsTriggerContext());
        cb_access::SharedValueContexts()->Add(new CityBotsValueContext());

        CbLog::Info("registered Citizen contexts into all class registries.");
        CityBotsRuntime::MarkContextsRegistered();

        CitizenRosterRegistry::Instance().LoadFromDatabase();
        CbCitizenAccountMgr::ValidateStageCast();
        CityPopulationMgr::RefreshFromWorld();
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        CbSettings::InvalidateConfCache();
        CityPopulationMgr::RefreshFromWorld();
    }

private:
    bool _registered = false;
};

class CityBotsPresencePlayerScript : public PlayerScript
{
public:
    CityBotsPresencePlayerScript()
        : PlayerScript("CityBotsPresencePlayerScript", {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_LOGOUT,
            PLAYERHOOK_ON_MAP_CHANGED,
            PLAYERHOOK_ON_UPDATE_ZONE,
            PLAYERHOOK_ON_UPDATE_AREA
        })
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        CityPopulationMgr::OnPlayerLogin(player);

        if (!player || !player->GetSession() || !player->GetSession()->IsBot())
            return;

        if (CitizenRosterRegistry::Instance().IsRosterGuid(player->GetGUID().GetCounter()))
        {
            // Finish bot login on the world thread immediately. RandomPlayerbotMgr
            // normally defers this through PlayerbotWorldThreadProcessor, which
            // stalls once duel hubs are active and the queue backs up.
            if (sRandomPlayerbotMgr.GetPlayerBot(player->GetGUID()) != player)
                sRandomPlayerbotMgr.OnBotLogin(player);

            CbCitizenLoginMgr::OnRosterBotLoggedIn(player->GetGUID().GetCounter());
            CbStrategyGate::ResetLoginStaging(player->GetGUID().GetCounter());
            CbStrategyGate::Reconcile(player);
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        CityPopulationMgr::OnPlayerLogout(player);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        CityPopulationMgr::OnPlayerMapChanged(player);

        if (player && player->GetSession() && player->GetSession()->IsBot())
            CbStrategyGate::Reconcile(player);
    }

    void OnPlayerUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        CityPopulationMgr::OnPlayerZoneUpdate(player, newZone);
    }

    void OnPlayerUpdateArea(Player* player, uint32 /*oldArea*/, uint32 newArea) override
    {
        CityPopulationMgr::OnPlayerAreaUpdate(player, newArea);
    }
};

class CityBotsReaperScript : public PlayerbotScript
{
public:
    CityBotsReaperScript() : PlayerbotScript("CityBotsReaperScript") {}

    void OnPlayerbotUpdate(uint32 diff) override
    {
        if (!CityBotsRuntime::ContextsRegistered())
            return;

        CbCitizenLoginMgr::Tick(diff);

        _gateSweepAccumMs += diff;
        if (_gateSweepAccumMs >= CB_STRATEGY_GATE_SWEEP_MS)
        {
            _gateSweepAccumMs = 0;
            if (CbCitizenLoginMgr::UsesDedicatedPool())
                CbStrategyGate::ReconcileRosterBots();
            else
                CbStrategyGate::ReconcileAllBots();
        }
    }

private:
    uint32 _gateSweepAccumMs = 0;
};

void AddSC_city_bots_module()
{
    new CityBotsRegistrarWorldScript();
    new CityBotsPresencePlayerScript();
    new CityBotsReaperScript();
}

#endif
