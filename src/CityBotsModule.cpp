/*
 * mod-city-bots — CityBotsModule.cpp
 */

#ifdef MOD_PLAYERBOTS

#include "CbCoreCompat.h"
#include "ScriptMgr.h"
#include "Log.h"
#include "Player.h"
#include "PlayerScript.h"

#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
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
            PLAYERHOOK_ON_UPDATE_AREA,
            PLAYERHOOK_ON_BEFORE_ACHI_COMPLETE,
            PLAYERHOOK_ON_BEFORE_CRITERIA_PROGRESS,
            PLAYERHOOK_CAN_INIT_TRADE,
            PLAYERHOOK_CAN_SET_TRADE_ITEM
        })
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        CityPopulationMgr::OnPlayerLogin(player);

        if (!player || !player->GetSession() || !CbIsBotSession(player->GetSession()))
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

    // Citizen bots run on their own accounts, so mod-playerbots' realm-first
    // guard in Playerbots.cpp does not cover them: RandomPlayerbotMgr::IsRandomBot()
    // only answers true for GUIDs on a random-bot account that are in the current
    // random pool. Without this hook the stage cast earns achievements like a real
    // player and can take the realm firsts on a live server (reported by Andood).
    bool OnPlayerBeforeAchievementComplete(Player* player, AchievementEntry const* /*achievement*/) override
    {
        return !IsBlockedCitizen(player);
    }

    // Blocking the criteria too keeps 400 bots from writing achievement progress
    // rows they can never complete.
    bool OnPlayerBeforeCriteriaProgress(Player* player, AchievementCriteriaEntry const* /*criteria*/) override
    {
        return !IsBlockedCitizen(player);
    }

    // mod-playerbots enforces AiPlayerbot.EnableRandomBotTrading inside
    // TradeStatusAction, behind the same IsRandomBot() test that missed the
    // achievement hook, so an owner who turned bot trading off still had 400
    // citizens trading with players. Enforce the owner's setting where the core
    // asks instead of inside a bot action the citizens never reach.
    // 0 = no trading, 1 = trading, 2 = the bot may only buy, 3 = only sell.
    bool OnPlayerCanInitTrade(Player* player, Player* target) override
    {
        if (CbCfgEnableRandomBotTrading(sPlayerbotAIConfig) != 0)
            return true;

        bool const targetIsCitizen = IsCitizenBot(target);
        if (!targetIsCitizen && !IsCitizenBot(player))
            return true;

        // Say why, the way TradeStatusAction does -- through the same text key, so
        // a server that customised trade_disabled gets its own wording here too.
        // A silently dead trade window reads as a broken bot.
        if (targetIsCitizen)
            target->Whisper(PlayerbotTextMgr::instance().GetBotTextOrDefault(
                                "trade_disabled", "Trading is disabled", {}),
                            LANG_UNIVERSAL, player);

        return false;
    }

    bool OnPlayerCanSetTradeItem(Player* player, Item* /*tradedItem*/, uint8 tradeSlot) override
    {
        int32 const mode = CbCfgEnableRandomBotTrading(sPlayerbotAIConfig);
        if (mode != 2 && mode != 3)
            return true;

        if (!player)
            return true;

        // The enchant slot is never handed over, and playerbots' own 2/3 checks
        // ignore it: CalculateCost only sums slots below TRADE_SLOT_TRADED_COUNT.
        // Blocking it here would break enchanting and craft-show for no gain.
        if (tradeSlot == TRADE_SLOT_NONTRADED)
            return true;

        Player* other = player->GetTrader();
        if (!other)
            return true;

        // 2 (only buy): the citizen must not hand its own items over.
        if (mode == 2 && IsCitizenBot(player) && !IsCitizenBot(other))
            return false;

        // 3 (only sell): the player must not hand items to a citizen.
        if (mode == 3 && IsCitizenBot(other) && !IsCitizenBot(player))
            return false;

        return true;
    }

    void OnPlayerLogout(Player* player) override
    {
        CityPopulationMgr::OnPlayerLogout(player);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        CityPopulationMgr::OnPlayerMapChanged(player);

        if (player && player->GetSession() && CbIsBotSession(player->GetSession()))
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

private:
    static bool IsCitizenBot(Player* player)
    {
        if (!player || !player->GetSession() || !CbIsBotSession(player->GetSession()))
            return false;

        return CitizenRosterRegistry::Instance().IsRosterGuid(player->GetGUID().GetCounter());
    }

    // Order matters, not for correctness but for cost: registering the criteria
    // hook puts this on every SetCriteriaProgress call by every character on the
    // realm. IsCitizenBot is a session flag plus a hash lookup and rejects humans
    // and random bots immediately; CbSettings::GetBool scans the settings table,
    // so it must only run for the citizens that got that far.
    static bool IsBlockedCitizen(Player* player)
    {
        return IsCitizenBot(player) && CbSettings::GetBool("BlockAchievements");
    }
};

// Ticks once per world update. This used to be a PlayerbotScript
// (OnPlayerbotUpdate), but newer mod-playerbots cores removed PlayerbotScript
// (the playerbots tick is now its own WorldScript::OnUpdate), and WorldScript
// exists on every core, old and new.
class CityBotsReaperScript : public WorldScript
{
public:
    CityBotsReaperScript() : WorldScript("CityBotsReaperScript") {}

    void OnUpdate(uint32 diff) override
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
