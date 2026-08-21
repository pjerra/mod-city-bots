#include "CbStrategyGate.h"

#include "CbCitizenAccountMgr.h"
#include "CbCitizenLoginMgr.h"
#include "CbCitizenRoles.h"
#include "CbCitizenStateAccess.h"
#include "CitizenRosterRegistry.h"
#include "CityBotsRuntime.h"

#include "Ai/City/CityBots/CbValueKeys.h"
#include "Ai/City/CityBots/CitizenInfo.h"
#include "Ai/City/CityBots/Data/CityLocationRegistry.h"
#include "Ai/City/CityBots/Data/CityPoiRegistry.h"
#include "Ai/City/CityBots/Data/CrowdDialogues.h"
#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "BotActivityRegistry.h"
#include "CbCitizenStateAccess.h"
#include "CbDuelBotUtil.h"
#include "CbLog.h"
#include "CbPlayerbotsIntegration.h"

#include "CharmInfo.h"
#include "CreatureAI.h"
#include "DatabaseEnv.h"
#include "AiObjectContext.h"
#include "DBCStores.h"
#include "Event.h"
#include "Item.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PathGenerator.h"
#include "Opcodes.h"
#include "PetDefines.h"
#include "Player.h"
#include "PlayerbotFactory.h"
#include "QueryResult.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellMgr.h"
#include "TemporarySummon.h"
#include "SocialMgr.h"
#include "StringFormat.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include "Playerbots.h"
#include "PlayerbotAI.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <initializer_list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    std::unordered_set<uint32> s_outfitApplied;
    std::unordered_set<uint32> s_duelerPrepared;
    std::unordered_set<uint32> s_duelerStrategiesLogged;
    std::unordered_map<uint32, uint32> s_directDuelMs;
    std::unordered_map<uint32, uint32> s_directMoveMs;
    std::unordered_map<uint32, uint32> s_directEmoteMs;
    std::unordered_map<uint32, uint32> s_directFishMs;
    std::unordered_map<uint32, uint32> s_dancerGreetMs;
    std::unordered_map<uint32, uint32> s_goldshireRouteStep;
    std::unordered_map<uint32, bool> s_stormwindDirectLastIssuedWasTrade;
    std::unordered_map<uint32, CitizenState> s_directRosterStates;
    std::unordered_set<uint32> s_rosterStatePrimed;
    std::unordered_map<uint32, uint32> s_directMountDecisionPoiId;
    std::unordered_map<uint32, uint32> s_directMountAttemptMs;
    std::unordered_map<uint32, uint32> s_pathRefuseStreak;

    // Sink forensics: the last MovePoint the gate issued per bot, and the
    // bot's z at the previous sweep. When a bot's z drops >8y between two
    // sweeps without a teleport, we log exactly which order it was following.
    struct LastMove
    {
        char const* source{"?"};
        float fromX{0}, fromY{0}, fromZ{0};
        float toX{0}, toY{0}, toZ{0};
        uint32 pathType{0};
        uint32 atMs{0};
    };
    std::unordered_map<uint32, LastMove> s_lastMove;
    std::unordered_map<uint32, float> s_lastSweepZ;

    void RecordMove(Player* bot, char const* source, float toX, float toY, float toZ,
                    uint32 pathType)
    {
        LastMove& m = s_lastMove[bot->GetGUID().GetCounter()];
        m.source = source;
        m.fromX = bot->GetPositionX();
        m.fromY = bot->GetPositionY();
        m.fromZ = bot->GetPositionZ();
        m.toX = toX; m.toY = toY; m.toZ = toZ;
        m.pathType = pathType;
        m.atMs = getMSTime();
    }

    void SinkForensics(Player* bot)
    {
        uint32 const gl = bot->GetGUID().GetCounter();
        float const z = bot->GetPositionZ();
        auto it = s_lastSweepZ.find(gl);
        if (it != s_lastSweepZ.end() && !bot->IsBeingTeleported() && it->second - z > 8.0f)
        {
            LastMove const& m = s_lastMove[gl];
            // 8-15y between two 3s sweeps is a ramp (Debug); >15y is a plunge
            // (Warn) -- the water tables sit 13-35y under the floors. The Warn
            // line that found the sink read "last order poi ... to
            // (x,y,59.5) pathType 4" in a z=95 district.
            float const drop = it->second - z;
            std::string const msg = Acore::StringFormat(
                "SINK {} (guid {}): z {:.1f} -> {:.1f} at ({:.1f},{:.1f}) | last order {} "
                "{}ms ago from ({:.1f},{:.1f},{:.1f}) to ({:.1f},{:.1f},{:.1f}) pathType {} "
                "| moving={} water={}",
                bot->GetName(), gl, it->second, z, bot->GetPositionX(),
                bot->GetPositionY(), m.source, GetMSTimeDiffToNow(m.atMs),
                m.fromX, m.fromY, m.fromZ, m.toX, m.toY, m.toZ, m.pathType,
                bot->isMoving() ? 1 : 0, bot->IsInWater() ? 1 : 0);
            // Warn only for a plunge INTO water (the class that matters) or a
            // 20y+ drop; Orgrimmar's ramps legitimately descend up to ~18y
            // between two sweeps and were 84 of 94 "plunges" in round 17.
            if (bot->IsInWater() || drop > 20.0f)
                CbLog::Warn("{}", msg);
            else
                CbLog::Debug("{}", msg);
        }
        s_lastSweepZ[gl] = z;
    }
    std::unordered_map<uint32, uint32> s_duelCooldownMs;
    std::unordered_map<uint32, uint32> s_duelStateMs;
    std::unordered_map<uint32, uint64> s_duelSignature;
    std::unordered_map<uint32, uint32> s_duelAttackMs;
    std::unordered_map<uint32, uint32> s_duelCompanionMs;
    std::unordered_map<uint32, uint32> s_duelCombatToolMs;
    std::unordered_map<uint32, uint32> s_duelOneSidedMs;
    std::unordered_map<uint32, uint32> s_lastDuelOpponent;
    std::unordered_map<uint32, uint32> s_previousDuelOpponent;
    std::unordered_map<uint32, uint32> s_duelFlourishMs;
    std::unordered_map<uint32, uint32> s_duelRepositionMs;
    std::unordered_map<uint32, uint32> s_duelReservedBy;
    std::unordered_map<uint32, uint32> s_duelReserveMs;
    std::unordered_map<uint32, uint32> s_duelProposer;
    std::unordered_map<uint32, float> s_duelMeetX;
    std::unordered_map<uint32, float> s_duelMeetY;
    std::unordered_map<uint32, float> s_duelMeetZ;
    std::unordered_set<uint32> s_wasDueling;
    std::unordered_set<uint32> s_missingStateWarned;
    std::unordered_set<uint32> s_stageCastPreparedLogged;
    std::unordered_set<uint32> s_companionSanitizedLogged;
    std::unordered_set<uint32> s_dancerLogged;
    std::unordered_set<uint32> s_duelLogged;
    std::unordered_set<uint32> s_duelBlockedLogged;
    constexpr float kGateDuelerRoamRadius = 40.0f;
    constexpr float kGoldshireDuelerRoamRadius = 28.0f;
    constexpr float kDuelerReturnDistance = 45.0f;
    constexpr uint32 kDuelCompleteCooldownMs = 10 * IN_MILLISECONDS;
    constexpr uint32 kDuelChallengeTimeoutMs = 12 * IN_MILLISECONDS;
    constexpr uint32 kDuelCountdownTimeoutMs = 15 * IN_MILLISECONDS;
    constexpr uint32 kDuelIdleTimeoutMs = 20 * IN_MILLISECONDS;
    constexpr uint32 kDuelReserveTimeoutMs = 20 * IN_MILLISECONDS;
    constexpr uint32 kDuelFlourishCooldownMs = 12 * IN_MILLISECONDS;
    constexpr uint32 kDuelRepositionCooldownMs = 6 * IN_MILLISECONDS;
    constexpr uint32 kGoldshireRoamTravelTimeoutMs = 22 * IN_MILLISECONDS;
    constexpr uint32 kCityRoamTravelTimeoutMs = 35 * IN_MILLISECONDS;
    char const* const kNonCombat = "citizen";
    char const* const kCombat = "citizen combat";

    bool AcceptDuelChallenge(Player* bot);
    void EnsureDuelerCombatForm(Player* bot);
    void EnsureDuelerCompanion(Player* bot, PlayerbotAI* botAI);
    void CommandDuelerPetAttack(Player* bot, Unit* target);
    void AddStrategyIfMissing(PlayerbotAI* botAI, char const* strat, BotState state);
    void RemoveStrategyIfPresent(PlayerbotAI* botAI, char const* strat, BotState state);
    void SuppressRogueDuelStealth(Player* bot, PlayerbotAI* botAI = nullptr);
    bool Throttle(std::unordered_map<uint32, uint32>& stamps, uint32 guidLow, uint32 intervalMs);

    class CbPlayerbotAISilenceAccess : public PlayerbotAI
    {
    public:
        void SilenceQueuedChat(time_t quietUntil)
        {
            chatCommands.clear();
            chatReplies.clear();
            currentChat.second = quietUntil;
        }
    };

    char const* const kStripMaintenance[] = {
        "grind", "new rpg", "rpg", "quest", "chat", "travel", "stay",
        "follow", "default", "loot", "gather", "pvp", "buff", "mount",
        "emote", "nc", "food", "boost", "avoid aoe", "formation",
        "dps assist", "cure", "save mana", "healer dps", "tank",
        "suggest dungeon", "suggest trade"
    };
    char const* const kRestoreNonCombat[] = { "grind", "new rpg", "quest" };

    bool IsCitizenAccount(uint32 accountId)
    {
        return CbCitizenAccountMgr::IsCitizenAccount(accountId);
    }

    bool IsRndBotAccount(uint32 accountId)
    {
        QueryResult result = PlayerbotsDatabase.Query(
            "SELECT 1 FROM playerbots_account_type WHERE account_id = {} AND account_type = 1",
            accountId);
        return result != nullptr;
    }

    uint32 CitizenHash(uint32 accountId, uint32 guidLow)
    {
        return (accountId ^ guidLow ^ 0xC17E) % 100;
    }

    void ApplyHomeLocation(Player* bot, CitizenState& state)
    {
        auto& locReg = CityLocationRegistry::Instance();
        uint32 const guidLow = bot->GetGUID().GetCounter();

        CitizenRosterEntry const* roster =
            CitizenRosterRegistry::Instance().FindByGuid(guidLow);
        if (roster)
        {
            state.homeLocationKey = roster->homeKey;
            locReg.StoreHomeLocation(guidLow, roster->homeKey);
            CityLocationDef const* loc = locReg.FindByKey(roster->homeKey);
            if (loc)
            {
                state.homeZoneId = loc->zoneId;
                if (loc->duelHub)
                    CbDuelBotUtil::EnsureLevel(bot);
            }
            return;
        }

        if (IsCitizenAccount(bot->GetSession()->GetAccountId()))
        {
            std::string stored =
                locReg.GetStoredHomeLocation(bot->GetGUID().GetCounter());
            CityLocationDef const* storedLoc = stored.empty()
                ? nullptr
                : locReg.FindByKey(stored);
            if (storedLoc)
            {
                state.homeLocationKey = stored;
                state.homeZoneId = storedLoc->zoneId;
                if (storedLoc->duelHub)
                    CbDuelBotUtil::EnsureLevel(bot);
                return;
            }
        }
        else if (state.homeLocationKey.empty())
        {
            state.homeLocationKey = bot->GetTeamId() == TEAM_HORDE ? "Orgrimmar" : "Stormwind";
        }

        CityLocationDef const* loc = locReg.FindByKey(state.homeLocationKey);
        if (loc)
            state.homeZoneId = loc->zoneId;
    }

    void ApplySpawnJitter(Player* bot, WorldPosition& pos)
    {
        uint32 const guidLow = bot->GetGUID().GetCounter();
        float const radius = static_cast<float>((guidLow % 10) + 2) * 0.75f;
        float const angle =
            static_cast<float>(guidLow % 360) * static_cast<float>(M_PI) / 180.0f;
        pos.setX(pos.GetPositionX() + std::cos(angle) * radius);
        pos.setY(pos.GetPositionY() + std::sin(angle) * radius);
    }

    bool NeedsStageCastOutfit(Player* bot)
    {
        if (!bot)
            return false;

        return !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BODY) ||
               !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS) ||
               !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET);
    }

    bool HasBasicStageSeed(Player* bot)
    {
        if (!bot)
            return false;

        Item* body = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_BODY);
        Item* legs = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_LEGS);
        Item* feet = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_FEET);

        return (body && body->GetEntry() == 38) ||
               (legs && legs->GetEntry() == 39) ||
               (feet && feet->GetEntry() == 40);
    }

    bool EquipIfEmpty(Player* bot, uint8 slot, uint32 itemId)
    {
        if (!bot || bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            return false;

        uint16 dest = 0;
        if (bot->CanEquipNewItem(slot, dest, itemId, false) != EQUIP_ERR_OK)
            return false;

        return bot->EquipNewItem(dest, itemId, true) != nullptr;
    }

    bool EquipItemReplacingSlot(Player* bot, uint8 slot, uint32 itemId)
    {
        if (!bot)
            return false;

        if (Item* equipped = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            if (equipped->GetEntry() == itemId)
                return true;

            bot->DestroyItem(INVENTORY_SLOT_BAG_0, slot, true);
        }

        uint16 dest = 0;
        if (bot->CanEquipNewItem(slot, dest, itemId, false) != EQUIP_ERR_OK)
            return false;

        return bot->EquipNewItem(dest, itemId, true) != nullptr;
    }

    uint32 StarterWeaponForClass(uint8 cls)
    {
        switch (cls)
        {
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                return 35; // Bent Staff
            case CLASS_PALADIN:
            case CLASS_PRIEST:
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                return 36; // Worn Mace
            case CLASS_WARRIOR:
            case CLASS_HUNTER:
            case CLASS_ROGUE:
            default:
                return 25; // Worn Shortsword
        }
    }

    bool IsDuelerRole(CitizenRole role)
    {
        return role == CITIZEN_ROLE_GATE_DUELIST ||
               role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;
    }

    int PreferredDuelerDpsSpecNo(Player* bot)
    {
        if (!bot)
            return 0;

        switch (bot->getClass())
        {
            case CLASS_WARRIOR:
                return 1; // Fury
            case CLASS_PALADIN:
                return 2; // Retribution
            case CLASS_HUNTER:
                return 1; // Marksmanship
            case CLASS_ROGUE:
                return 1; // Combat
            case CLASS_PRIEST:
                return 2; // Shadow
            case CLASS_DEATH_KNIGHT:
                return 2; // Unholy
            case CLASS_SHAMAN:
                return 1; // Enhancement
            case CLASS_MAGE:
                return 2; // Frost
            case CLASS_WARLOCK:
                return 2; // Destruction
            case CLASS_DRUID:
                return 1; // Feral, later filtered to non-tank/cat
            default:
                return 0;
        }
    }

    bool IsAllowedDuelerDpsSpec(Player* bot)
    {
        if (!bot)
            return false;

        uint32 constexpr warlockMetamorphosis = 47241;
        if (bot->getClass() == CLASS_WARLOCK &&
            (bot->HasSpell(warlockMetamorphosis) || bot->HasAura(warlockMetamorphosis)))
            return false;

        if (!PlayerbotAI::IsDps(bot, true) ||
            PlayerbotAI::IsTank(bot, true) ||
            PlayerbotAI::IsHeal(bot, true))
            return false;

        if (bot->getClass() == CLASS_DRUID)
            return PlayerbotAI::IsMelee(bot, true);

        return true;
    }

    bool IsRangedDueler(Player* bot)
    {
        if (!bot)
            return false;

        switch (bot->getClass())
        {
            case CLASS_HUNTER:
            case CLASS_MAGE:
            case CLASS_PRIEST:
            case CLASS_WARLOCK:
                return true;
            default:
                return false;
        }
    }

    bool IsMeleeDueler(Player* bot)
    {
        if (!bot || IsRangedDueler(bot))
            return false;

        switch (bot->getClass())
        {
            case CLASS_ROGUE:
            case CLASS_WARRIOR:
            case CLASS_DEATH_KNIGHT:
            case CLASS_PALADIN:
                return true;
            case CLASS_SHAMAN:
            case CLASS_DRUID:
                return PlayerbotAI::IsMelee(bot, true);
            default:
                return PlayerbotAI::IsMelee(bot, true);
        }
    }

    void EnsureDuelerDpsSpec(Player* bot)
    {
        if (!bot)
            return;

        int const specNo = PreferredDuelerDpsSpecNo(bot);
        PlayerbotFactory::InitTalentsBySpecNo(bot, specNo, true);

        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
            botAI->ResetStrategies(false);

        EnsureDuelerCombatForm(bot);

        if (!IsAllowedDuelerDpsSpec(bot))
            CbLog::Warn("city dueler {} (guid {}) still reports non-DPS after forcing spec {}",
                        bot->GetName(), bot->GetGUID().GetCounter(), specNo);
    }

    void EnsureDuelerCombatForm(Player* bot)
    {
        if (!bot)
            return;

        uint32 constexpr warlockMetamorphosis = 47241;
        if (bot->getClass() == CLASS_WARLOCK && bot->HasAura(warlockMetamorphosis))
            bot->RemoveAura(warlockMetamorphosis);

        if (bot->getClass() != CLASS_DRUID ||
            PlayerbotAI::IsTank(bot, true) || PlayerbotAI::IsHeal(bot, true))
            return;

        uint32 constexpr catForm = 768;
        if (!bot->HasAura(catForm) && !bot->IsNonMeleeSpellCast(false))
            bot->CastSpell(bot, catForm, false);
    }

    void TryDuelerAction(PlayerbotAI* botAI, char const* action)
    {
        if (botAI && action)
            botAI->DoSpecificAction(action, Event(), true);
    }

    void TryDuelerSpell(PlayerbotAI* botAI, char const* spell, Unit* target)
    {
        if (!botAI || !spell || !target)
            return;

        if (botAI->CanCastSpell(spell, target))
            botAI->CastSpell(spell, target);
    }

    void CommandDuelerPetAttack(Player* bot, Unit* target)
    {
        if (!bot || !target || bot->getClass() != CLASS_HUNTER)
            return;

        Guardian* pet = bot->GetGuardianPet();
        if (!pet || !pet->IsAlive() || pet->GetMap() != target->GetMap())
            return;

        pet->SetReactState(REACT_DEFENSIVE);
        pet->ClearUnitState(UNIT_STATE_FOLLOW);
        pet->AttackStop();
        pet->SetTarget(target->GetGUID());

        if (CharmInfo* charmInfo = pet->GetCharmInfo())
        {
            charmInfo->SetCommandState(COMMAND_ATTACK);
            charmInfo->SetIsCommandAttack(true);
            charmInfo->SetIsAtStay(false);
            charmInfo->SetIsFollowing(false);
            charmInfo->SetIsCommandFollow(false);
            charmInfo->SetIsReturning(false);
        }

        if (pet->ToCreature() && pet->ToCreature()->AI())
            pet->ToCreature()->AI()->AttackStart(target);
    }

    void EnsureDuelerCompanion(Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || !botAI)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_duelCompanionMs, guidLow, 5 * IN_MILLISECONDS + (guidLow % 4) * IN_MILLISECONDS))
            return;

        switch (bot->getClass())
        {
            case CLASS_HUNTER:
                AddStrategyIfMissing(botAI, "pet", BOT_STATE_NON_COMBAT);
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "call pet");
                else
                    bot->GetPet()->SetReactState(REACT_DEFENSIVE);
                break;
            case CLASS_MAGE:
                AddStrategyIfMissing(botAI, "frost", BOT_STATE_COMBAT);
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "summon water elemental");
                break;
            case CLASS_WARLOCK:
                AddStrategyIfMissing(botAI, "pet", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "felhunter", BOT_STATE_NON_COMBAT);
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "summon felhunter");
                break;
            default:
                break;
        }
    }

    Player* FindConnectedRosterBot(uint32 guidLow)
    {
        ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(guidLow);
        Player* bot = ObjectAccessor::FindConnectedPlayer(guid);
        if (!bot || !bot->IsInWorld() || !bot->GetSession() || !bot->GetSession()->IsBot())
            return nullptr;

        return bot;
    }

    void SanitizeStageCastCompanions(Player* bot)
    {
        if (!bot || !CitizenRosterRegistry::Instance().IsRosterGuid(bot->GetGUID().GetCounter()))
            return;

        bool changed = false;
        if (bot->HasMountedAura())
        {
            bot->Dismount();
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
            changed = true;
        }

        CitizenRosterEntry const* entry =
            CitizenRosterRegistry::Instance().FindByGuid(bot->GetGUID().GetCounter());
        bool const keepCombatCompanion = entry && IsDuelerRole(entry->role);

        if (!keepCombatCompanion && bot->GetPet())
        {
            bot->RemovePet(nullptr, PET_SAVE_NOT_IN_SLOT, true);
            changed = true;
        }

        if (changed && s_companionSanitizedLogged.insert(bot->GetGUID().GetCounter()).second)
            CbLog::Info("cleared stage-cast mount/pet state for {} (guid {})",
                        bot->GetName(), bot->GetGUID().GetCounter());
    }

    void PrepareStageCastLevel(Player* bot, uint32 targetLevel)
    {
        if (!bot)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (s_stageCastPreparedLogged.insert(guidLow).second)
            CbLog::Info("preparing stage-cast bot {} (guid {}, target level {})",
                        bot->GetName(), guidLow, targetLevel);

        if (bot->GetLevel() < targetLevel)
            bot->GiveLevel(targetLevel);

        SanitizeStageCastCompanions(bot);
        bot->SetFullHealth();
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
    }

    void EnsureDuelerPrepared(Player* bot, CitizenRosterEntry const& entry)
    {
        if (!bot || !IsDuelerRole(entry.role))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (s_duelerPrepared.count(guidLow) != 0)
            return;

        uint32 const targetLevel = CITIZEN_DUEL_HUB_LEVEL;

        if (bot->GetLevel() < targetLevel)
            bot->GiveLevel(targetLevel);

        PrepareStageCastLevel(bot, targetLevel);
        EnsureDuelerDpsSpec(bot);
        {
            // Citizens can reach the hub with a freshly-created spellbook
            // (4-6 starting spells at level 80); the level boost alone never
            // taught their class spells, so duels ran on rank-1 scraps.
            PlayerbotFactory factory(bot, targetLevel);
            factory.InitClassSpells();
            factory.InitAvailableSpells();
        }
        if (bot->getClass() == CLASS_HUNTER && !bot->GetPet())
        {
            PlayerbotFactory factory(bot, targetLevel);
            factory.InitPet();
            factory.InitPetTalents();
        }
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        {
            botAI->DoSpecificAction("autogear", Event(), true);
            botAI->DoSpecificAction("equip upgrades packet action", Event(), true);
            EnsureDuelerCompanion(bot, botAI);
        }

        EquipIfEmpty(bot, EQUIPMENT_SLOT_MAINHAND, StarterWeaponForClass(bot->getClass()));
        EnsureDuelerCombatForm(bot);

        s_duelerPrepared.insert(guidLow);
    }

    void EnsureStageCastOutfit(Player* bot)
    {
        if (!bot || !CitizenRosterRegistry::Instance().IsRosterGuid(bot->GetGUID().GetCounter()))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        CitizenRosterEntry const* entry =
            CitizenRosterRegistry::Instance().FindByGuid(guidLow);
        if (entry && entry->role == CITIZEN_ROLE_INN_DANCER)
            return;
        if (entry && IsDuelerRole(entry->role))
        {
            EnsureDuelerPrepared(bot, *entry);
            return;
        }

        if (entry && (NeedsStageCastOutfit(bot) || HasBasicStageSeed(bot)) &&
            !s_outfitApplied.count(guidLow))
        {
            PrepareStageCastLevel(bot, std::max<uint32>(bot->GetLevel(), entry->level));
            s_outfitApplied.insert(guidLow);
            return;
        }

        if (!NeedsStageCastOutfit(bot) && s_outfitApplied.count(guidLow))
            return;

        EquipIfEmpty(bot, EQUIPMENT_SLOT_BODY, 38);        // Recruit's Shirt
        EquipIfEmpty(bot, EQUIPMENT_SLOT_LEGS, 39);        // Recruit's Pants
        EquipIfEmpty(bot, EQUIPMENT_SLOT_FEET, 40);        // Recruit's Boots
        EquipIfEmpty(bot, EQUIPMENT_SLOT_MAINHAND, StarterWeaponForClass(bot->getClass()));

        s_outfitApplied.insert(guidLow);
    }

    void StripPlayerbotsMaintenance(PlayerbotAI* botAI)
    {
        if (!botAI)
            return;

        for (char const* strat : kStripMaintenance)
        {
            if (botAI->HasStrategy(strat, BOT_STATE_NON_COMBAT))
                botAI->ChangeStrategy(std::string("-") + strat, BOT_STATE_NON_COMBAT);

            if (botAI->HasStrategy(strat, BOT_STATE_COMBAT))
                botAI->ChangeStrategy(std::string("-") + strat, BOT_STATE_COMBAT);
        }
    }

    void AddStrategyIfMissing(PlayerbotAI* botAI, char const* strat, BotState state)
    {
        if (botAI && strat && !botAI->HasStrategy(strat, state))
            botAI->ChangeStrategy(std::string("+") + strat, state);
    }

    void RemoveStrategyIfPresent(PlayerbotAI* botAI, char const* strat, BotState state)
    {
        if (botAI && strat && botAI->HasStrategy(strat, state))
            botAI->ChangeStrategy(std::string("-") + strat, state);
    }

    void ApplyDuelerCombatStrategies(Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || !botAI)
            return;

        char const* const combatAdds[] = {
            "default", "cast time", "boost", "racials", "dps assist", "aoe",
            "duel", "pvp"
        };
        char const* const combatRemoves[] = {
            "stay", "follow", "passive", "flee", "threat"
        };

        for (char const* strat : combatAdds)
            AddStrategyIfMissing(botAI, strat, BOT_STATE_COMBAT);

        for (char const* strat : combatRemoves)
            RemoveStrategyIfPresent(botAI, strat, BOT_STATE_COMBAT);

        switch (bot->getClass())
        {
            case CLASS_MAGE:
                RemoveStrategyIfPresent(botAI, "fire", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "firestarter", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "arcane", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "frost", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "bdps", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "dps", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cc", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cure", BOT_STATE_COMBAT);
                break;
            case CLASS_WARLOCK:
                RemoveStrategyIfPresent(botAI, "demo", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "meta melee", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "affli", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "felguard", BOT_STATE_NON_COMBAT);
                RemoveStrategyIfPresent(botAI, "spellstone", BOT_STATE_NON_COMBAT);
                AddStrategyIfMissing(botAI, "destro", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "boost", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "curse of elements", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "curse of agony", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "curse of tongues", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "curse of exhaustion", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cc", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "pet", BOT_STATE_COMBAT);
                RemoveStrategyIfPresent(botAI, "imp", BOT_STATE_NON_COMBAT);
                AddStrategyIfMissing(botAI, "felhunter", BOT_STATE_NON_COMBAT);
                AddStrategyIfMissing(botAI, "firestone", BOT_STATE_NON_COMBAT);
                AddStrategyIfMissing(botAI, "ss self", BOT_STATE_NON_COMBAT);
                break;
            case CLASS_PRIEST:
                AddStrategyIfMissing(botAI, "dps", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "shadow", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "shadow debuff", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "shadow aoe", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cure", BOT_STATE_COMBAT);
                break;
            case CLASS_HUNTER:
                AddStrategyIfMissing(botAI, "mm", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "bdps", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cc", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "pet", BOT_STATE_NON_COMBAT);
                break;
            case CLASS_SHAMAN:
                AddStrategyIfMissing(botAI, "enh", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "windfury", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "strength of earth", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "magma", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cure", BOT_STATE_COMBAT);
                break;
            case CLASS_DRUID:
                AddStrategyIfMissing(botAI, "cat", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "feral charge", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cc", BOT_STATE_COMBAT);
                break;
            case CLASS_WARRIOR:
                AddStrategyIfMissing(botAI, "fury", BOT_STATE_COMBAT);
                break;
            case CLASS_ROGUE:
                SuppressRogueDuelStealth(bot, botAI);
                break;
            case CLASS_PALADIN:
                AddStrategyIfMissing(botAI, "dps", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "cure", BOT_STATE_COMBAT);
                break;
            case CLASS_DEATH_KNIGHT:
                AddStrategyIfMissing(botAI, "unholy", BOT_STATE_COMBAT);
                AddStrategyIfMissing(botAI, "unholy aoe", BOT_STATE_COMBAT);
                break;
            default:
                AddStrategyIfMissing(botAI, "dps", BOT_STATE_COMBAT);
                break;
        }

        if (s_duelerStrategiesLogged.insert(bot->GetGUID().GetCounter()).second)
            CbLog::Info("applied arena-style combat strategies to city dueler {} (guid {})",
                        bot->GetName(), bot->GetGUID().GetCounter());

        EnsureDuelerCompanion(bot, botAI);
    }

    void SuppressRogueDuelStealth(Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || bot->getClass() != CLASS_ROGUE)
            return;

        bot->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
        bot->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);

        if (!botAI)
            botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        RemoveStrategyIfPresent(botAI, "dps", BOT_STATE_COMBAT);
        RemoveStrategyIfPresent(botAI, "melee", BOT_STATE_COMBAT);
        RemoveStrategyIfPresent(botAI, "boost", BOT_STATE_COMBAT);
        RemoveStrategyIfPresent(botAI, "stealth", BOT_STATE_COMBAT);
        RemoveStrategyIfPresent(botAI, "stealthed", BOT_STATE_COMBAT);
        RemoveStrategyIfPresent(botAI, "stealth", BOT_STATE_NON_COMBAT);
        RemoveStrategyIfPresent(botAI, "stealthed", BOT_STATE_NON_COMBAT);
        TryDuelerAction(botAI, "unstealth");
    }

    void SuppressPlayerbotsChannelReplies(PlayerbotAI* botAI)
    {
        if (!botAI || !botAI->GetAiObjectContext())
            return;

        // Packet-level chat reactions do not require the "chat" strategy.
        // Roster city bots should only speak through their city actions.
        time_t const quietUntil = time(nullptr) + 24 * 60 * 60;
        botAI->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(quietUntil);
        botAI->GetAiObjectContext()->GetValue<time_t>("last said", "say")->Set(quietUntil);
        botAI->GetAiObjectContext()->GetValue<time_t>("last said", "general")->Set(quietUntil);
        botAI->GetAiObjectContext()->GetValue<time_t>("last said", "trade")->Set(quietUntil);
        botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(false);
        reinterpret_cast<CbPlayerbotAISilenceAccess*>(botAI)->SilenceQueuedChat(quietUntil);
    }

    void EnsureDuelStrategy(PlayerbotAI* botAI, bool shouldBeCitizen,
                            bool duelHub, bool goldshireDuelist)
    {
        if (!botAI || !shouldBeCitizen || (!duelHub && !goldshireDuelist))
            return;

        if (!botAI->HasStrategy("duel", BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy("+duel", BOT_STATE_NON_COMBAT);
    }

    void ResolveTeleportPosition(Player* bot, CityPoi const& poi, WorldPosition& pos)
    {
        if (poi.poiType == CITY_POI_INN_BED)
            return;

        // pos carries the (possibly jittered) x/y and the POI's curated z.
        // In WMO cities the terrain -- and even a flat water table -- continues
        // UNDERNEATH the walkable floors. When the jittered x/y sat where the
        // real floor is above the search seed, UpdateAllowedPositionZ resolved
        // to that under-city surface (seven Orgrimmar citizens at the water
        // plane's constant z=-2.8) and TeleportTo planted the bot below the
        // mesh, where mmaps have no polygons -- the "noclip under the city"
        // class. The curated POI z is authoritative: a resolve that lands far
        // from it means the jittered x/y is not on the intended floor, so use
        // the exact POI point instead of trusting the resolve.
        float const poiZ = pos.GetPositionZ();
        float x = pos.GetPositionX();
        float y = pos.GetPositionY();
        float z = poiZ + 2.0f;
        bot->UpdateAllowedPositionZ(x, y, z);
        if (std::fabs(z - poiZ) > 5.0f)
        {
            // The jittered point is on another level. Before falling back to
            // the EXACT curated point -- which for some POIs sits inside a
            // wall or a step below the floor (the "standing inside walls / a
            // bit underground" look after a rescue) -- try a few closer
            // jitters that resolve on-level and dry.
            WorldPosition const exact = CityPoiRegistry::Instance().GetWorldPosition(poi);
            for (int attempt = 0; attempt < 4; ++attempt)
            {
                float const a = frand(0.0f, 2.0f * static_cast<float>(M_PI));
                float const d = frand(1.5f, 4.0f);
                float jx = exact.GetPositionX() + std::cos(a) * d;
                float jy = exact.GetPositionY() + std::sin(a) * d;
                float jz = poiZ + 2.0f;
                bot->UpdateAllowedPositionZ(jx, jy, jz);
                if (std::fabs(jz - poiZ) > 3.0f)
                    continue;
                if (bot->GetMap() &&
                    bot->GetMap()->IsInWater(bot->GetPhaseMask(), jx, jy, jz, bot->GetCollisionHeight()))
                    continue;
                pos.setX(jx);
                pos.setY(jy);
                pos.setZ(jz);
                return;
            }
            CbLog::Info("teleport resolve rejected for {} at poi {} ({:.1f},{:.1f}): "
                        "resolved z {:.1f} vs curated {:.1f} -- using exact poi point",
                        bot->GetName(), poi.id, x, y, z, poiZ);
            pos.setX(exact.GetPositionX());
            pos.setY(exact.GetPositionY());
            pos.setZ(exact.GetPositionZ());
            return;
        }
        pos.setZ(z);
    }

    // A citizen standing well BELOW its stage POI while horizontally near it
    // is under the mesh (the failure ResolveTeleportPosition now prevents, or
    // a leftover from before the fix). Pathfinding cannot recover from there;
    // a re-teleport through the fixed resolver can. Bots merely traveling far
    // from their POI stay untouched (2D distance gate).
    bool IsSunkenBelowStage(Player* bot, CitizenState& state)
    {
        if (!state.targetPoi.IsValid() || bot->GetMapId() != state.targetPoi.GetMapId())
            return false;

        float const dx = bot->GetPositionX() - state.targetPoi.GetPositionX();
        float const dy = bot->GetPositionY() - state.targetPoi.GetPositionY();
        if (dx * dx + dy * dy > 60.0f * 60.0f)
            return false;

        return state.targetPoi.GetPositionZ() - bot->GetPositionZ() > 10.0f;
    }

    // Like IsSunkenBelowStage but symmetric: a citizen 10y BELOW its target is
    // under the mesh, 10y ABOVE it is hovering on a straight-line spline that
    // never had a walkable path (both seen live in Orgrimmar 2026-08-16, the
    // first sweep after movement was un-frozen). Either way pathfinding cannot
    // recover from there; a re-stage through the fixed resolver can.
    bool IsOffStagePlane(Player* bot, CitizenState& state)
    {
        // A bot mid-route can legitimately be far below (climbing) or above
        // (descending) its destination — only a STANDING bot off the plane is
        // stuck.
        if (bot->isMoving())
            return false;

        // Reference floor = the LOWEST curated POI of the home city within 60y,
        // not the travel target: a bot walking from POI A to a far POI B that
        // sinks halfway ends 13-20y under the POIs it is standing beside — the
        // target-keyed net (60y around B) never saw it (5 bots on the
        // Orgrimmar water table per boot, rounds 5-6). Lowest-within-radius,
        // not nearest: a multi-level city has POIs on each tier, so a citizen
        // legitimately on the lower tier is still at or above SOME nearby POI
        // (the nearest-POI version yanked 243 healthy bots per boot in
        // Undercity/Ironforge/Dalaran). A real sink is below all of them.
        // Margin 25y, not 10: the 10y version yanked 6,569 healthy Undercity
        // citizens in one day (their -62 tier sits 19y under the -43 tier's
        // POIs, which are within 60y) -- the "standing inside walls / a bit
        // underground" look is a bot teleported onto the exact POI point by a
        // false rescue. No multi-tier city has tiers 25y apart without POIs
        // on the lower one; the real sinks (Orgrimmar -2.8 under floor 10-18,
        // Stormwind 59.5 under 95) are 13-35y down and are ALSO caught by the
        // water clause in the caller, which is the primary net now.
        if (!state.homeLocationKey.empty())
        {
            float minZ = 0.0f;
            if (CityPoiRegistry::Instance().LowestPoiZWithin(
                    state.homeLocationKey, bot->GetPositionX(), bot->GetPositionY(),
                    60.0f, &minZ) &&
                bot->GetPositionZ() < minZ - 25.0f)
                return true;
        }

        if (!state.targetPoi.IsValid() || bot->GetMapId() != state.targetPoi.GetMapId())
            return false;

        float const dx = bot->GetPositionX() - state.targetPoi.GetPositionX();
        float const dy = bot->GetPositionY() - state.targetPoi.GetPositionY();
        if (dx * dx + dy * dy > 60.0f * 60.0f)
            return false;

        // Below-only here too: "above the target" is a legitimate upper tier
        // more often than a hover, and hovering bots fall to the floor by
        // themselves — sunken ones do not.
        return state.targetPoi.GetPositionZ() - bot->GetPositionZ() > 10.0f;
    }

    void TeleportToHome(Player* bot, CitizenState& state)
    {
        ApplyHomeLocation(bot, state);
        CbCitizenRoles::ApplyRosterRole(bot, state);

        CityPoi const* poi = nullptr;
        if (state.targetPoiId)
            poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId);

        if (!poi)
        {
            CityPoiType spawnType = CITY_POI_INN;
            CityLocationDef const* loc =
                CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
            if (loc && loc->duelHub)
                spawnType = CITY_POI_DUEL_RING;
            else if (loc && loc->socialHub)
                spawnType = CITY_POI_TAVERN;

            poi = !state.homeLocationKey.empty()
                ? CityPoiRegistry::Instance().PickPoi(state.homeLocationKey, spawnType)
                : nullptr;
            if (!poi && state.homeZoneId)
                poi = CityPoiRegistry::Instance().PickPoi(state.homeZoneId, spawnType);
        }

        if (!poi)
            return;

        WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
        if (poi->poiType != CITY_POI_INN_BED)
            ApplySpawnJitter(bot, pos);
        ResolveTeleportPosition(bot, *poi, pos);
        bot->TeleportTo(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                        pos.GetOrientation());
        state.targetPoi = pos;
        state.targetPoiId = poi->id;
        state.atPoi = true;

        if (state.role == CITIZEN_ROLE_INN_DANCER)
        {
            CbCitizenRoles::StripForInnDance(bot);
            state.strippedForDance = true;
        }
    }

    void EnsureDedicatedStagePosition(Player* bot, CitizenState& state)
    {
        if (!bot || bot->duel || bot->IsInCombat() || !CbCitizenRoles::IsDedicated(state.role) ||
            IsDuelerRole(state.role))
            return;

        CityPoi const* poi = state.targetPoiId
            ? CityPoiRegistry::Instance().GetPoi(state.targetPoiId)
            : nullptr;
        if (!poi)
            return;

        WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
        float const dist = bot->GetDistance(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
        float const maxDist = state.role == CITIZEN_ROLE_INN_DANCER
            ? 1.5f
            : (state.role == CITIZEN_ROLE_GATE_DUELIST
                ? kDuelerReturnDistance
                : (state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST
                    ? 12.0f
                    : (state.role == CITIZEN_ROLE_PLAZA_TALKER
                        ? 80.0f
                        // Crowd citizens WALK between their bank/AH pair; a 20y
                        // leash would teleport-snap them mid-wander. 150y covers
                        // both cities' pairs while still rescuing a lost bot.
                        : (state.role == CITIZEN_ROLE_CROWD ? 150.0f : 20.0f))));
        if (dist <= maxDist && bot->GetMapId() == pos.GetMapId())
            return;

        if (poi->poiType != CITY_POI_INN_BED)
            ApplySpawnJitter(bot, pos);
        ResolveTeleportPosition(bot, *poi, pos);
        bot->TeleportTo(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(),
                        pos.GetOrientation());
        state.targetPoi = pos;
        state.targetPoiId = poi->id;
        state.atPoi = true;

        if (state.role == CITIZEN_ROLE_INN_DANCER)
        {
            CbCitizenRoles::StripForInnDance(bot);
            state.strippedForDance = true;
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

    bool IsDuelCooldownActive(Player* player)
    {
        if (!player)
            return false;

        uint32 const guidLow = player->GetGUID().GetCounter();
        auto itr = s_duelCooldownMs.find(guidLow);
        return itr != s_duelCooldownMs.end() &&
               GetMSTimeDiffToNow(itr->second) < kDuelCompleteCooldownMs;
    }

    bool Throttle(std::unordered_map<uint32, uint32>& stamps, uint32 guidLow, uint32 intervalMs);
    bool MoveTowardDuelOpponent(Player* bot, CitizenState& state, Player* opponent);
    bool IsInsideDuelArea(Player* player, CitizenState const& state, float radius);
    void PrepareForMovement(Player* bot, CitizenState& state);

    char const* DuelStateName(DuelState state)
    {
        switch (state)
        {
            case DUEL_STATE_CHALLENGED:
                return "challenged";
            case DUEL_STATE_COUNTDOWN:
                return "countdown";
            case DUEL_STATE_IN_PROGRESS:
                return "in_progress";
            case DUEL_STATE_COMPLETED:
                return "completed";
            default:
                return "unknown";
        }
    }

    void ClearDuelReservation(uint32 guidLow)
    {
        s_duelReservedBy.erase(guidLow);
        s_duelReserveMs.erase(guidLow);
        s_duelProposer.erase(guidLow);
        s_duelMeetX.erase(guidLow);
        s_duelMeetY.erase(guidLow);
        s_duelMeetZ.erase(guidLow);

        for (auto itr = s_duelReservedBy.begin(); itr != s_duelReservedBy.end();)
        {
            if (itr->second == guidLow)
            {
                s_duelReserveMs.erase(itr->first);
                s_duelProposer.erase(itr->first);
                s_duelMeetX.erase(itr->first);
                s_duelMeetY.erase(itr->first);
                s_duelMeetZ.erase(itr->first);
                itr = s_duelReservedBy.erase(itr);
            }
            else
                ++itr;
        }
    }

    void ClearDuelTracking(uint32 guidLow)
    {
        s_duelStateMs.erase(guidLow);
        s_duelSignature.erase(guidLow);
        s_duelAttackMs.erase(guidLow);
        s_duelCombatToolMs.erase(guidLow);
        s_duelOneSidedMs.erase(guidLow);
        s_duelRepositionMs.erase(guidLow);
        ClearDuelReservation(guidLow);
    }

    void RestoreDuelerVitals(Player* bot)
    {
        if (!bot)
            return;

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

        if (bot->IsAlive())
            bot->SetFullHealth();

        if (bot->GetMaxPower(POWER_MANA) > 0)
            bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
        if (bot->GetMaxPower(POWER_ENERGY) > 0)
            bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));
        if (bot->GetMaxPower(POWER_FOCUS) > 0)
            bot->SetPower(POWER_FOCUS, bot->GetMaxPower(POWER_FOCUS));
        if (bot->GetMaxPower(POWER_RAGE) > 0)
            bot->SetPower(POWER_RAGE, 0);
        if (bot->GetMaxPower(POWER_RUNIC_POWER) > 0)
            bot->SetPower(POWER_RUNIC_POWER, 0);

        if (Pet* pet = bot->GetPet())
            if (pet->IsAlive())
                pet->SetFullHealth();
    }

    void ReserveDuelPair(uint32 challengerGuid, uint32 opponentGuid,
                         float meetX, float meetY, float meetZ)
    {
        uint32 const now = getMSTime();
        s_duelReservedBy[challengerGuid] = opponentGuid;
        s_duelReserveMs[challengerGuid] = now;
        s_duelProposer[challengerGuid] = challengerGuid;
        s_duelMeetX[challengerGuid] = meetX;
        s_duelMeetY[challengerGuid] = meetY;
        s_duelMeetZ[challengerGuid] = meetZ;

        s_duelReservedBy[opponentGuid] = challengerGuid;
        s_duelReserveMs[opponentGuid] = now;
        s_duelProposer[opponentGuid] = challengerGuid;
        s_duelMeetX[opponentGuid] = meetX;
        s_duelMeetY[opponentGuid] = meetY;
        s_duelMeetZ[opponentGuid] = meetZ;
    }

    bool IsDuelReservedByOther(uint32 candidateGuid, uint32 requesterGuid)
    {
        auto reserve = s_duelReservedBy.find(candidateGuid);
        if (reserve == s_duelReservedBy.end())
            return false;

        uint32 const reservedAt = s_duelReserveMs[candidateGuid];
        if (!reservedAt || GetMSTimeDiffToNow(reservedAt) > kDuelReserveTimeoutMs)
        {
            ClearDuelReservation(candidateGuid);
            return false;
        }

        return reserve->second != requesterGuid;
    }

    void RecordDuelPair(uint32 guidLow, uint32 opponentGuid)
    {
        if (!guidLow || !opponentGuid)
            return;

        s_previousDuelOpponent[guidLow] = s_lastDuelOpponent[guidLow];
        s_previousDuelOpponent[opponentGuid] = s_lastDuelOpponent[opponentGuid];
        s_lastDuelOpponent[guidLow] = opponentGuid;
        s_lastDuelOpponent[opponentGuid] = guidLow;
    }

    void MaybeDuelFlourish(Player* bot)
    {
        if (!bot || bot->duel || bot->IsInCombat() || bot->IsBeingTeleported())
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_duelFlourishMs, guidLow, kDuelFlourishCooldownMs + (guidLow % 5) * IN_MILLISECONDS))
            return;

        switch (urand(0, 5))
        {
            case 0:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                break;
            case 1:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
                break;
            case 2:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                break;
            case 3:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                break;
            case 4:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_FLEX);
                break;
            default:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                break;
        }
    }

    void CancelCityDuel(Player* bot, char const* reason)
    {
        if (!bot || !bot->duel)
            return;

        Player* opponent = bot->duel->Opponent;
        uint32 const guidLow = bot->GetGUID().GetCounter();
        uint32 const opponentGuid = opponent ? opponent->GetGUID().GetCounter() : 0;
        DuelState const state = bot->duel->State;

        CbLog::Info("resetting stale city duel for {} (guid {}, state {}, opponent {} guid {}): {}",
                    bot->GetName(), guidLow, DuelStateName(state),
                    opponent ? opponent->GetName() : "none", opponentGuid,
                    reason ? reason : "stale");

        bot->CombatStopWithPets(true);
        if (opponent)
            opponent->CombatStopWithPets(true);

        if (opponent)
            bot->DuelComplete(DUEL_INTERRUPTED);

        RestoreDuelerVitals(bot);
        if (opponent)
            RestoreDuelerVitals(opponent);

        uint32 const now = getMSTime();
        s_duelCooldownMs[guidLow] = now;
        s_directDuelMs.erase(guidLow);
        s_directMoveMs.erase(guidLow);
        ClearDuelTracking(guidLow);

        if (opponentGuid)
        {
            s_duelCooldownMs[opponentGuid] = now;
            s_directDuelMs.erase(opponentGuid);
            s_directMoveMs.erase(opponentGuid);
            ClearDuelTracking(opponentGuid);
        }
    }

    void RestoreDueler(Player* bot)
    {
        if (!bot)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!bot->IsAlive() || bot->isDead())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }

        bot->CombatStopWithPets(true);
        RestoreDuelerVitals(bot);
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        bot->ClearInCombat();
        s_directMoveMs.erase(guidLow);
    }

    bool RecoverDeadDueler(Player* bot, CitizenState& state)
    {
        if (!bot || (bot->IsAlive() && !bot->isDead()))
            return false;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        Player* opponent = bot->duel ? bot->duel->Opponent : nullptr;
        uint32 const opponentGuid = opponent ? opponent->GetGUID().GetCounter() : 0;

        if (bot->duel)
            CancelCityDuel(bot, "dueler died");

        RestoreDueler(bot);
        if (opponent && !opponent->IsAlive())
            RestoreDueler(opponent);

        uint32 const now = getMSTime();
        s_duelCooldownMs[guidLow] = now;
        s_directDuelMs.erase(guidLow);
        s_directMoveMs.erase(guidLow);
        ClearDuelTracking(guidLow);
        if (opponentGuid)
        {
            s_duelCooldownMs[opponentGuid] = now;
            s_directDuelMs.erase(opponentGuid);
            s_directMoveMs.erase(opponentGuid);
            ClearDuelTracking(opponentGuid);
        }

        EnsureDedicatedStagePosition(bot, state);
        CbLog::Info("revived city dueler {} (guid {}) after non-alive duel state",
                    bot->GetName(), guidLow);
        return true;
    }

    void ForceDuelAttack(Player* attacker, Player* target)
    {
        if (!attacker || !target || !attacker->IsAlive() || !target->IsAlive() ||
            attacker->GetMap() != target->GetMap() || !attacker->IsWithinDistInMap(target, 30.0f))
            return;

        attacker->SetSelection(target->GetGUID());
        SuppressRogueDuelStealth(attacker);
        if (IsMeleeDueler(attacker))
            attacker->Attack(target, true);
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(attacker))
        {
            if (attacker->getClass() != CLASS_ROGUE)
                botAI->DoSpecificAction("attack duel opponent", Event(), true);
            if (attacker->getClass() == CLASS_ROGUE)
            {
                TryDuelerAction(botAI, "sprint");
                TryDuelerSpell(botAI, "shadowstep", target);
                TryDuelerSpell(botAI, "kick", target);
                TryDuelerSpell(botAI, "kidney shot", target);
                TryDuelerSpell(botAI, "cheap shot", target);
                TryDuelerSpell(botAI, "mutilate", target);
                TryDuelerSpell(botAI, "sinister strike", target);
                TryDuelerSpell(botAI, "hemorrhage", target);
                TryDuelerSpell(botAI, "eviscerate", target);
            }
            if (attacker->getClass() == CLASS_HUNTER)
                botAI->DoSpecificAction("pet attack", Event(), true);
        }
        CommandDuelerPetAttack(attacker, target);
    }

    void PulseDuelerCombatTools(Player* bot, Player* opponent)
    {
        if (!bot || !opponent || !bot->IsAlive() || !opponent->IsAlive() ||
            bot->IsNonMeleeSpellCast(false) || !bot->IsWithinDistInMap(opponent, 35.0f))
            return;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        EnsureDuelerCompanion(bot, botAI);
        SuppressRogueDuelStealth(bot, botAI);

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_duelCombatToolMs, guidLow, 4 * IN_MILLISECONDS + (guidLow % 5) * IN_MILLISECONDS))
            return;

        bot->SetSelection(opponent->GetGUID());
        switch (bot->getClass())
        {
            case CLASS_ROGUE:
                TryDuelerAction(botAI, "sprint");
                TryDuelerAction(botAI, "evasion");
                TryDuelerAction(botAI, "cloak of shadows");
                TryDuelerSpell(botAI, "shadowstep", opponent);
                TryDuelerSpell(botAI, "kick", opponent);
                TryDuelerSpell(botAI, "kidney shot", opponent);
                TryDuelerSpell(botAI, "blind", opponent);
                TryDuelerSpell(botAI, "mutilate", opponent);
                TryDuelerSpell(botAI, "sinister strike", opponent);
                TryDuelerSpell(botAI, "hemorrhage", opponent);
                TryDuelerSpell(botAI, "eviscerate", opponent);
                break;
            case CLASS_HUNTER:
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "call pet");
                CommandDuelerPetAttack(bot, opponent);
                TryDuelerAction(botAI, "pet attack");
                TryDuelerAction(botAI, "rapid fire");
                TryDuelerAction(botAI, "kill command");
                TryDuelerSpell(botAI, "silencing shot", opponent);
                if (bot->GetDistance(opponent) < 9.0f)
                    TryDuelerAction(botAI, "freezing trap");
                break;
            case CLASS_MAGE:
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "summon water elemental");
                TryDuelerAction(botAI, "icy veins");
                TryDuelerAction(botAI, "mirror image");
                TryDuelerAction(botAI, "ice barrier");
                TryDuelerSpell(botAI, "counterspell", opponent);
                if (bot->GetDistance(opponent) < 12.0f)
                {
                    TryDuelerSpell(botAI, "frost nova", opponent);
                    TryDuelerSpell(botAI, "deep freeze", opponent);
                }
                break;
            case CLASS_WARLOCK:
                if (!bot->GetPet())
                    TryDuelerAction(botAI, "summon felhunter");
                TryDuelerAction(botAI, "shadow ward");
                TryDuelerSpell(botAI, "curse of agony", opponent);
                TryDuelerSpell(botAI, "curse of tongues", opponent);
                TryDuelerSpell(botAI, "curse of exhaustion", opponent);
                TryDuelerSpell(botAI, "shadowfury", opponent);
                TryDuelerSpell(botAI, "chaos bolt", opponent);
                TryDuelerSpell(botAI, "conflagrate", opponent);
                TryDuelerSpell(botAI, "fear", opponent);
                break;
            default:
                break;
        }
    }

    bool MoveToRangedDuelDistance(Player* bot, CitizenState& state, Player* opponent)
    {
        if (!bot || !opponent || !IsRangedDueler(bot) || bot->IsBeingTeleported() ||
            bot->IsNonMeleeSpellCast(false))
            return false;

        float const currentDistance = bot->GetDistance(opponent);
        if (currentDistance >= 11.0f && currentDistance <= 20.0f)
            return false;

        float const preferredDistance = bot->getClass() == CLASS_HUNTER ? 16.0f : 14.0f;
        float dx = bot->GetPositionX() - opponent->GetPositionX();
        float dy = bot->GetPositionY() - opponent->GetPositionY();
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 0.1f)
        {
            float const angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
            dx = std::cos(angle);
            dy = std::sin(angle);
            len = 1.0f;
        }

        float targetX = opponent->GetPositionX() + (dx / len) * preferredDistance;
        float targetY = opponent->GetPositionY() + (dy / len) * preferredDistance;
        float targetZ = opponent->GetPositionZ();

        float const anchorX = state.targetPoi.GetPositionX();
        float const anchorY = state.targetPoi.GetPositionY();
        float const anchorZ = state.targetPoi.GetPositionZ();
        float const leash = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;
        float const ax = targetX - anchorX;
        float const ay = targetY - anchorY;
        float const distFromAnchor = std::sqrt(ax * ax + ay * ay);
        if (distFromAnchor > leash && distFromAnchor > 0.1f)
        {
            float const scale = leash / distFromAnchor;
            targetX = anchorX + ax * scale;
            targetY = anchorY + ay * scale;
            targetZ = anchorZ;
        }

        bot->UpdateAllowedPositionZ(targetX, targetY, targetZ);
        PrepareForMovement(bot, state);
        RecordMove(bot, "duel-ranged", targetX, targetY, targetZ, 0);
        bot->GetMotionMaster()->MovePoint(0, targetX, targetY, targetZ);
        return true;
    }

    void MaybeRepositionDuringDuel(Player* bot, CitizenState& state, Player* opponent)
    {
        if (!bot || !opponent || bot->IsBeingTeleported() || bot->IsNonMeleeSpellCast(false))
            return;

        if (MoveToRangedDuelDistance(bot, state, opponent))
            return;

        if (!IsMeleeDueler(bot) || !bot->IsWithinDistInMap(opponent, 8.0f))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_duelRepositionMs, guidLow, kDuelRepositionCooldownMs + (guidLow % 4) * IN_MILLISECONDS))
            return;

        if (urand(0, 99) >= 35)
            return;

        MoveTowardDuelOpponent(bot, state, opponent);
    }

    bool DriveActiveDuel(Player* bot, CitizenState& state)
    {
        if (!bot || !bot->duel)
            return bot && bot->IsInCombat();

        uint32 const guidLow = bot->GetGUID().GetCounter();
        Player* opponent = bot->duel->Opponent;
        uint32 const opponentGuid = opponent ? opponent->GetGUID().GetCounter() : 0;
        DuelState const duelState = bot->duel->State;
        uint64 const signature =
            (static_cast<uint64>(opponentGuid) << 8) | static_cast<uint8>(duelState);
        uint32 const now = getMSTime();

        if (s_duelSignature[guidLow] != signature)
        {
            s_duelSignature[guidLow] = signature;
            s_duelStateMs[guidLow] = now;
        }

        uint32 const age = GetMSTimeDiffToNow(s_duelStateMs[guidLow]);

        if (!opponent || !opponent->IsInWorld())
        {
            CancelCityDuel(bot, "missing opponent");
            return true;
        }

        if (!IsAllowedDuelerDpsSpec(bot) || !IsAllowedDuelerDpsSpec(opponent))
        {
            CancelCityDuel(bot, "non-DPS dueler spec");
            return true;
        }

        EnsureDuelerCombatForm(bot);
        EnsureDuelerCombatForm(opponent);
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot))
        {
            EnsureDuelerCompanion(bot, botAI);
            SuppressRogueDuelStealth(bot, botAI);
        }
        if (PlayerbotAI* opponentAI = GET_PLAYERBOT_AI(opponent))
        {
            EnsureDuelerCompanion(opponent, opponentAI);
            SuppressRogueDuelStealth(opponent, opponentAI);
        }

        if (!bot->IsAlive() || !opponent->IsAlive())
        {
            CancelCityDuel(bot, !bot->IsAlive() ? "dueler non-alive" : "opponent non-alive");
            RestoreDueler(bot);
            RestoreDueler(opponent);
            s_duelCooldownMs[guidLow] = now;
            if (opponentGuid)
                s_duelCooldownMs[opponentGuid] = now;
            return true;
        }

        float const roam = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;
        if (!IsInsideDuelArea(bot, state, roam + 15.0f) ||
            !IsInsideDuelArea(opponent, state, roam + 15.0f))
        {
            CancelCityDuel(bot, "outside duel area");
            return true;
        }

        if (duelState == DUEL_STATE_CHALLENGED && age > kDuelChallengeTimeoutMs)
        {
            CancelCityDuel(bot, "challenge timed out");
            return true;
        }

        if (duelState == DUEL_STATE_CHALLENGED &&
            CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
        {
            AcceptDuelChallenge(opponent);
            if (!bot->IsWithinDistInMap(opponent, 8.0f) &&
                Throttle(s_directMoveMs, guidLow, 1 * IN_MILLISECONDS))
            {
                MoveTowardDuelOpponent(bot, state, opponent);
                MoveTowardDuelOpponent(opponent, state, bot);
            }
            return true;
        }

        if (duelState == DUEL_STATE_COUNTDOWN && age > kDuelCountdownTimeoutMs)
        {
            CancelCityDuel(bot, "countdown timed out");
            return true;
        }

        if (duelState == DUEL_STATE_COUNTDOWN)
        {
            if (Throttle(s_duelRepositionMs, guidLow, 1 * IN_MILLISECONDS))
            {
                MoveToRangedDuelDistance(bot, state, opponent);
                if (CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
                    MoveToRangedDuelDistance(opponent, state, bot);
            }
            return true;
        }

        if (duelState == DUEL_STATE_IN_PROGRESS)
        {
            PulseDuelerCombatTools(bot, opponent);
            if (CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
                PulseDuelerCombatTools(opponent, bot);

            float const distance = bot->GetDistance(opponent);
            bool const botAttackingOpponent =
                bot->GetVictim() == opponent;
            bool const opponentAttackingBot =
                opponent->GetVictim() == bot;
            bool const botEngaged =
                botAttackingOpponent ||
                (IsRangedDueler(bot) && distance <= 45.0f &&
                 (bot->IsInCombat() || bot->IsNonMeleeSpellCast(false)));
            bool const opponentEngaged =
                opponentAttackingBot ||
                (IsRangedDueler(opponent) && distance <= 45.0f &&
                 (opponent->IsInCombat() || opponent->IsNonMeleeSpellCast(false)));

            if (distance < 10.0f && Throttle(s_duelRepositionMs, guidLow, 1 * IN_MILLISECONDS))
            {
                bool movedRanged = MoveToRangedDuelDistance(bot, state, opponent);
                if (CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
                    movedRanged = MoveToRangedDuelDistance(opponent, state, bot) || movedRanged;
                if (movedRanged)
                    return true;
            }

            if (distance > 5.5f && Throttle(s_directMoveMs, guidLow + 700000u, 1 * IN_MILLISECONDS))
            {
                if (IsMeleeDueler(bot))
                    MoveTowardDuelOpponent(bot, state, opponent);
                if (CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid) &&
                    IsMeleeDueler(opponent))
                    MoveTowardDuelOpponent(opponent, state, bot);
            }

            if (!bot->IsWithinLOSInMap(opponent) && age > 8 * IN_MILLISECONDS)
            {
                CancelCityDuel(bot, "in-progress duel lost line of sight");
                return true;
            }

            if (distance > 24.0f && age > 10 * IN_MILLISECONDS)
            {
                CancelCityDuel(bot, "in-progress duel stayed too far apart");
                return true;
            }

            if (!bot->IsWithinDistInMap(opponent, 18.0f))
            {
                if (Throttle(s_directMoveMs, guidLow, 1 * IN_MILLISECONDS))
                {
                    ForceDuelAttack(bot, opponent);
                    ForceDuelAttack(opponent, bot);
                    if (!MoveToRangedDuelDistance(bot, state, opponent))
                        MoveTowardDuelOpponent(bot, state, opponent);
                    if (CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
                    {
                        if (!MoveToRangedDuelDistance(opponent, state, bot))
                            MoveTowardDuelOpponent(opponent, state, bot);
                    }
                }
            }
            else if ((!botAttackingOpponent || !opponentAttackingBot) &&
                     Throttle(s_duelAttackMs, guidLow, 2 * IN_MILLISECONDS))
            {
                ForceDuelAttack(bot, opponent);
                ForceDuelAttack(opponent, bot);
                if (!botAttackingOpponent && IsMeleeDueler(bot) && distance > 4.0f)
                    MoveTowardDuelOpponent(bot, state, opponent);
                if (!opponentAttackingBot && IsMeleeDueler(opponent) && distance > 4.0f &&
                    CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid))
                    MoveTowardDuelOpponent(opponent, state, bot);
            }
            else
                MaybeRepositionDuringDuel(bot, state, opponent);

            bool const botMeleeStuck =
                IsMeleeDueler(bot) && !botAttackingOpponent && opponentEngaged;
            bool const opponentMeleeStuck =
                CitizenRosterRegistry::Instance().IsRosterGuid(opponentGuid) &&
                IsMeleeDueler(opponent) && !opponentAttackingBot && botEngaged;
            if (botMeleeStuck || opponentMeleeStuck)
            {
                uint32 const stuckKey = botMeleeStuck ? guidLow : opponentGuid;
                uint32& stuckSince = s_duelOneSidedMs[stuckKey];
                if (!stuckSince)
                    stuckSince = now;
                else if (GetMSTimeDiffToNow(stuckSince) > 10 * IN_MILLISECONDS)
                {
                    CancelCityDuel(bot, botMeleeStuck
                        ? "melee dueler failed to engage"
                        : "opponent melee dueler failed to engage");
                    return true;
                }
            }
            else
            {
                s_duelOneSidedMs.erase(guidLow);
                if (opponentGuid)
                    s_duelOneSidedMs.erase(opponentGuid);
            }

            if (!botEngaged && !opponentEngaged && age > kDuelIdleTimeoutMs)
            {
                CancelCityDuel(bot, "in-progress duel had no attackers");
                return true;
            }
        }

        return true;
    }

    bool IsInsideDuelArea(Player* player, CitizenState const& state, float radius)
    {
        if (!player || player->GetMapId() != state.targetPoi.GetMapId())
            return false;

        return player->GetDistance(state.targetPoi.GetPositionX(),
                                   state.targetPoi.GetPositionY(),
                                   state.targetPoi.GetPositionZ()) <= radius;
    }

    char const* DirectDuelBlockReason(Player* bot, Player* opponent, CitizenRole role)
    {
        if (!bot || !opponent || bot == opponent)
            return "missing opponent";

        if (!opponent->GetSession())
            return "opponent has no session";

        if (!bot->IsAlive() || !opponent->IsAlive())
            return "dead";

        if (!IsAllowedDuelerDpsSpec(bot) || !IsAllowedDuelerDpsSpec(opponent))
            return "non-DPS spec";

        if (bot->GetTeamId() != opponent->GetTeamId())
            return "wrong team";

        if (bot->duel || opponent->duel || bot->IsInCombat() || opponent->IsInCombat())
            return "busy";

        if (IsDuelCooldownActive(bot) || IsDuelCooldownActive(opponent))
            return "cooling down";

        if (IsDuelReservedByOther(bot->GetGUID().GetCounter(), opponent->GetGUID().GetCounter()) ||
            IsDuelReservedByOther(opponent->GetGUID().GetCounter(), bot->GetGUID().GetCounter()))
            return "reserved";

        if (!CanDuelHere(bot) || !CanDuelHere(opponent))
            return "duels not allowed here";

        if (!bot->IsWithinLOSInMap(opponent))
            return "no line of sight";

        if (opponent->GetSocial() && opponent->GetSocial()->HasIgnore(bot->GetGUID()))
            return "ignored";

        if (bot->GetLevel() != CITIZEN_DUEL_HUB_LEVEL ||
            opponent->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
            return "wrong level";

        return nullptr;
    }

    bool IsDuelReservationUsable(Player* bot, Player* opponent, CitizenRole role)
    {
        if (!bot || !opponent || bot == opponent)
            return false;

        if (!bot->IsAlive() || !opponent->IsAlive())
            return false;

        if (!bot->GetSession() || !opponent->GetSession())
            return false;

        if (!IsAllowedDuelerDpsSpec(bot) || !IsAllowedDuelerDpsSpec(opponent))
            return false;

        if (bot->GetTeamId() != opponent->GetTeamId())
            return false;

        if (bot->duel || opponent->duel || bot->IsInCombat() || opponent->IsInCombat())
            return false;

        if (IsDuelCooldownActive(bot) || IsDuelCooldownActive(opponent))
            return false;

        if (!CanDuelHere(bot) || !CanDuelHere(opponent))
            return false;

        if (bot->GetLevel() != CITIZEN_DUEL_HUB_LEVEL ||
            opponent->GetLevel() != CITIZEN_DUEL_HUB_LEVEL)
            return false;

        (void)role;
        return true;
    }

    bool CanDirectDuelWith(Player* bot, Player* opponent, CitizenRole role)
    {
        return !DirectDuelBlockReason(bot, opponent, role);
    }

    bool AcceptDuelChallenge(Player* bot)
    {
        if (!bot || !bot->duel || !bot->GetSession())
            return false;

        if (bot == bot->duel->Initiator || bot->duel->State != DUEL_STATE_CHALLENGED)
            return false;

        WorldPacket packet(CMSG_DUEL_ACCEPTED, 8);
        packet << bot->GetGuidValue(PLAYER_DUEL_ARBITER);
        bot->GetSession()->HandleDuelAcceptedOpcode(packet);
        return true;
    }

    bool AcceptPendingDuel(Player* bot, CitizenState const& state)
    {
        if (!bot || !IsDuelerRole(state.role))
            return false;

        return AcceptDuelChallenge(bot);
    }

    Player* FindDirectDuelOpponent(Player* bot, CitizenState const& state)
    {
        if (!bot || !IsDuelerRole(state.role) || state.homeLocationKey.empty())
            return nullptr;

        Player* best = nullptr;
        Player* fallbackRecent = nullptr;
        Player* fallbackPrevious = nullptr;
        float bestDist = state.role == CITIZEN_ROLE_GATE_DUELIST ? 100.0f : 60.0f;
        float bestScore = bestDist + 8.0f;
        float fallbackRecentDist = bestDist;
        float fallbackPreviousDist = bestDist;
        char const* firstBlock = nullptr;
        std::string firstBlockName;
        uint32 const guidLow = bot->GetGUID().GetCounter();
        uint32 const lastOpponent = s_lastDuelOpponent[guidLow];
        uint32 const previousOpponent = s_previousDuelOpponent[guidLow];

        auto consider = [&](Player* other)
        {
            if (!other)
                return;

            if (char const* reason = DirectDuelBlockReason(bot, other, state.role))
            {
                if (!firstBlock)
                {
                    firstBlock = reason;
                    firstBlockName = other->GetName();
                }
                return;
            }

            float const dist = bot->GetDistance(other);
            uint32 const otherGuid = other->GetGUID().GetCounter();
            if (dist <= 18.0f && otherGuid != lastOpponent)
            {
                best = other;
                bestScore = -1.0f;
                return;
            }

            if (otherGuid == lastOpponent)
            {
                if (dist < fallbackRecentDist)
                {
                    fallbackRecent = other;
                    fallbackRecentDist = dist;
                }
                return;
            }

            if (otherGuid == previousOpponent)
            {
                if (dist < fallbackPreviousDist)
                {
                    fallbackPrevious = other;
                    fallbackPreviousDist = dist;
                }
                return;
            }

            float const score = dist + frand(0.0f, 8.0f);
            if (score < bestScore)
            {
                best = other;
                bestScore = score;
            }
        };

        for (CitizenRosterEntry const* entry :
             CitizenRosterRegistry::Instance().ByHome(state.homeLocationKey.c_str()))
        {
            if (!entry || entry->guid == bot->GetGUID().GetCounter() ||
                entry->role != state.role)
                continue;

            Player* other = FindConnectedRosterBot(entry->guid);
            if (!other)
                continue;

            consider(other);
        }

        float const areaRadius = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            (void)guid;
            if (!player || player == bot || !player->IsInWorld() ||
                !player->GetSession() || player->GetSession()->IsBot())
                continue;
            if (!IsInsideDuelArea(player, state, areaRadius))
                continue;

            consider(player);
        }

        if (!best)
            best = fallbackPrevious;
        if (!best)
            best = fallbackRecent;

        if (!best && firstBlock && s_duelBlockedLogged.insert(bot->GetGUID().GetCounter()).second)
            CbLog::Warn("dueler {} (guid {}) found no usable opponent near {}; first blocked candidate: {} ({})",
                        bot->GetName(), bot->GetGUID().GetCounter(),
                        state.homeLocationKey, firstBlockName, firstBlock);

        return best;
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

            float const dist = bot->GetDistance(player);
            if (dist <= bestDist)
            {
                best = player;
                bestDist = dist;
            }
        }

        return best;
    }

    bool Throttle(std::unordered_map<uint32, uint32>& stamps, uint32 guidLow, uint32 intervalMs)
    {
        uint32 const last = stamps[guidLow];
        if (!last)
        {
            uint32 const jitter = intervalMs > 1
                ? ((guidLow * 1103515245u + 12345u) % intervalMs)
                : 0;
            uint32 const elapsed = intervalMs > jitter ? intervalMs - jitter : intervalMs;
            stamps[guidLow] = getMSTime() - elapsed;
            return false;
        }

        if (last && GetMSTimeDiffToNow(last) < intervalMs)
            return false;

        stamps[guidLow] = getMSTime();
        return true;
    }

    bool IsStormwindTradePoi(CityPoi const* poi)
    {
        return poi && poi->locationKey == "Stormwind" &&
               (poi->district == "Trade" || poi->district == "TradeDistrict");
    }

    bool IsOrgrimmarStrengthPoi(CityPoi const* poi)
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

        if (IsStormwindTradePoi(poi) || IsOrgrimmarStrengthPoi(poi))
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
            return !IsStormwindTradePoi(poi);

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

    void PrepareForMovement(Player* bot, CitizenState& state)
    {
        if (!bot)
            return;

        if (state.sitting)
        {
            bot->SetStandState(UNIT_STAND_STATE_STAND);
            state.sitting = false;
        }

        bot->ClearEmoteState();
        bot->GetMotionMaster()->Clear();
    }

    void DismountIfMounted(Player* bot)
    {
        if (!bot || !bot->HasMountedAura())
            return;

        bot->Dismount();
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
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

    bool CastCityTravelMount(Player* bot)
    {
        if (!bot || bot->HasMountedAura() || bot->IsNonMeleeSpellCast(false))
            return false;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
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

    void ClampTravelZ(Player* bot, float x, float y, float& z)
    {
        if (!bot)
            return;

        bot->UpdateAllowedPositionZ(x, y, z);

        float groundZ = INVALID_HEIGHT;
        float const walkZ = bot->GetMapWaterOrGroundLevel(x, y, z + 6.0f, &groundZ);
        if (walkZ > INVALID_HEIGHT && std::fabs(z - walkZ) > 4.0f)
            z = walkZ;
        else if (groundZ > INVALID_HEIGHT && std::fabs(z - groundZ) > 4.0f)
            z = groundZ;

        if (bot->GetMap())
        {
            float const mapZ = bot->GetMap()->GetHeight(
                bot->GetPhaseMask(), x, y, z + 6.0f, true, 20.0f);
            if (mapZ > INVALID_HEIGHT && std::fabs(z - mapZ) > 4.0f)
                z = mapZ;
        }

        if (z <= INVALID_HEIGHT)
            z = bot->GetPositionZ();
    }

    // True when a travel-z resolve crossed onto ANOTHER LEVEL relative to the
    // curated seed — a roof above, or the under-city water table below (WMO
    // cities have both under/over every floor). Callers doing SHORT wanders
    // around an anchor must SKIP such a destination (walking there sank 14+
    // bots onto Orgrimmar's z=-2.8 water plane, live 2026-08-17). NB: do NOT
    // rewrite z back to the seed instead — a rewritten z floats off the real
    // floor, the navmesh poly lookup fails, and every path gets refused (that
    // variant froze all 200 citizens for a night, measured 0/200 movers).
    bool ResolvedOntoAnotherLevel(float resolvedZ, float seedZ)
    {
        return std::fabs(resolvedZ - seedZ) > 6.0f;
    }

    // Complete-on-mesh test shared by every MovePoint the gate issues. Any
    // path type that ends off the mesh (NOPATH, SHORTCUT, NOT_USING_PATH,
    // INCOMPLETE, FARFROMPOLY_END) makes PointMovementGenerator finish with a
    // straight segment -- the exact "walk into the floor" sink.
    // NB round 9 tried refusing INCOMPLETE and FARFROMPOLY_END too — that
    // froze Orgrimmar (4/75 movers, 1112 refusals/boot) AND bots still sank,
    // so those flags were neither the sink nor safe to ban: on an imperfect
    // city mesh most legitimate routes are INCOMPLETE.
    // THE MECHANISM BEHIND EVERY "UNDER THE CITY" SINK (forensics over one
    // full day, 2026-08-19: 16,666 plunges, 10,884 on plain 'poi' orders with
    // sane on-floor destinations, 4,918 on mesh waypoints). AzerothCore's
    // PathGenerator includes NAV_WATER polygons for Players
    // (CreateFilter: "perfect support not possible, just stay safe"), so a
    // perfectly valid navmesh route across a city may run THROUGH the canal /
    // the Valley of Strength pond / the under-city water table. The bots were
    // never glitching -- they were swimming the shortest path, and at the
    // water table there is no walkable way back out. A route that touches
    // liquid at any point is refused for citizens (fishers never use these
    // helpers). The filter is private, so this is a post-check on the points.
    bool PathTouchesWater(Player* bot, PathGenerator const& path)
    {
        Map const* map = bot->GetMap();
        if (!map)
            return false;
        uint32 const phase = bot->GetPhaseMask();
        float const ch = bot->GetCollisionHeight();
        Movement::PointsArray const& pts = path.GetPath();
        for (G3D::Vector3 const& p : pts)
            if (map->IsInWater(phase, p.x, p.y, p.z, ch))
                return true;
        // Path points are several yards apart; also sample the midpoints so a
        // narrow canal cannot slip between two dry points. And an INCOMPLETE
        // path's straight tail starts at the last mesh point: if the water is
        // within a stride of it, the tail goes in (round 17: 10 water plunges
        // in 20 min, every one a type-4 route whose mesh part ended at the
        // pond's edge).
        for (std::size_t i = 1; i < pts.size(); ++i)
        {
            G3D::Vector3 const m = (pts[i - 1] + pts[i]) * 0.5f;
            if (map->IsInWater(phase, m.x, m.y, m.z, ch))
                return true;
        }
        if (!pts.empty() && (path.GetPathType() & PATHFIND_INCOMPLETE))
        {
            G3D::Vector3 const& e = pts.back();
            static float const kRing[][2] = { {5,0}, {-5,0}, {0,5}, {0,-5}, {3.5f,3.5f}, {-3.5f,3.5f}, {3.5f,-3.5f}, {-3.5f,-3.5f} };
            for (auto const& r : kRing)
                if (map->IsInWater(phase, e.x + r[0], e.y + r[1], e.z, ch))
                    return true;
        }
        return false;
    }

    bool HasCompleteWalkablePath(Player* bot, float x, float y, float z)
    {
        PathGenerator path(bot);
        if (!path.CalculatePath(x, y, z, false))
            return false;
        uint32 const type = path.GetPathType();
        if (type & (PATHFIND_NOPATH | PATHFIND_SHORTCUT | PATHFIND_NOT_USING_PATH))
            return false;
        if (type & PATHFIND_INCOMPLETE)
        {
            // Long straight tail = the walk-off-the-rim class (see MoveTowardPoi).
            G3D::Vector3 const& end = path.GetActualEndPosition();
            float const tx = end.x - x, ty = end.y - y, tz = end.z - z;
            if (tx * tx + ty * ty + tz * tz > 15.0f * 15.0f)
                return false;
        }
        if (PathTouchesWater(bot, path))
            return false;
        return true;
    }

    bool MaybeMountForPoiTravel(Player* bot, CitizenState& state)
    {
        if (!bot || bot->duel || bot->IsInCombat() || bot->HasMountedAura() ||
            !bot->IsOutdoors() || !state.targetPoiId)
            return false;

        if (IsDuelerRole(state.role) || state.role == CITIZEN_ROLE_INN_DANCER)
            return false;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (s_directMountDecisionPoiId[guidLow] == state.targetPoiId)
            return false;

        if (bot->GetDistance(state.targetPoi.GetPositionX(), state.targetPoi.GetPositionY(),
                             state.targetPoi.GetPositionZ()) < 20.0f)
            return false;

        s_directMountDecisionPoiId[guidLow] = state.targetPoiId;
        s_directMountAttemptMs[guidLow] = 0;
        uint32 const chance = CbSettings::GetUInt("MountBetweenPoiChance");
        if (chance && roll_chance_i(chance) && CastCityTravelMount(bot))
        {
            s_directMountAttemptMs[guidLow] = getMSTime();
        }

        return false;
    }

    bool MoveAroundTarget(Player* bot, CitizenState& state, float radius, uint32 intervalMs,
                          float returnDistance)
    {
        if (!bot || !state.targetPoi.IsValid())
            return false;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_directMoveMs, guidLow, intervalMs))
            return false;

        float const anchorX = state.targetPoi.GetPositionX();
        float const anchorY = state.targetPoi.GetPositionY();
        float const anchorZ = state.targetPoi.GetPositionZ();
        if (bot->GetDistance(anchorX, anchorY, anchorZ) > returnDistance)
        {
            if (!HasCompleteWalkablePath(bot, anchorX, anchorY, anchorZ))
                return false;
            PrepareForMovement(bot, state);
            RecordMove(bot, "wander-return", anchorX, anchorY, anchorZ, 0);
            bot->GetMotionMaster()->MovePoint(0, anchorX, anchorY, anchorZ);
            return true;
        }

        // Try several angles per interval: one random jitter per 14-37s
        // interval, silently refused when it resolves off-level or has no
        // complete path, left Orgrimmar's inn/plaza citizens standing for
        // whole boots with NOTHING in the log (round 14: 65/75 idle, 0 lines).
        for (int attempt = 0; attempt < 6; ++attempt)
        {
            float const angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
            float const distance = frand(radius * 0.35f, radius);
            float x = anchorX + std::cos(angle) * distance;
            float y = anchorY + std::sin(angle) * distance;
            float z = anchorZ;
            ClampTravelZ(bot, x, y, z);
            if (ResolvedOntoAnotherLevel(z, anchorZ))
                continue;   // off-ledge jitter — walking there leaves the level
            if (!HasCompleteWalkablePath(bot, x, y, z))
                continue;   // would end in a straight tail
            PrepareForMovement(bot, state);
            RecordMove(bot, "wander-around", x, y, z, 0);
            bot->GetMotionMaster()->MovePoint(0, x, y, z);
            return true;
        }
        static std::unordered_map<uint32, uint32> s_wanderRefuseLogMs;
        if (Throttle(s_wanderRefuseLogMs, guidLow, 120 * IN_MILLISECONDS))
            CbLog::Info("wander refused 6x for {} at ({:.1f},{:.1f},{:.1f}) around poi {} r={:.0f}",
                        bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                        bot->GetPositionZ(), state.targetPoiId, radius);
        return false;
    }

    bool ShuffleAroundCurrentPoi(Player* bot, CitizenState& state, float radius,
                                 float returnDistance)
    {
        if (!bot || !state.targetPoi.IsValid())
            return false;

        float const anchorX = state.targetPoi.GetPositionX();
        float const anchorY = state.targetPoi.GetPositionY();
        float const anchorZ = state.targetPoi.GetPositionZ();

        float x = anchorX;
        float y = anchorY;
        float z = anchorZ;
        if (bot->GetDistance(anchorX, anchorY, anchorZ) <= returnDistance)
        {
            float const angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
            float const distance = frand(radius * 0.35f, radius);
            x = anchorX + std::cos(angle) * distance;
            y = anchorY + std::sin(angle) * distance;
            ClampTravelZ(bot, x, y, z);
            if (ResolvedOntoAnotherLevel(z, anchorZ))
                return false;   // off-ledge jitter — skip this shuffle
        }

        if (!HasCompleteWalkablePath(bot, x, y, z))
            return false;
        PrepareForMovement(bot, state);
        RecordMove(bot, "wander-shuffle", x, y, z, 0);
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }

    bool MoveTowardDuelOpponent(Player* bot, CitizenState& state, Player* opponent)
    {
        if (!bot || !opponent || !state.targetPoi.IsValid())
            return false;

        // Real players may be duel opponents, but only bot sessions take
        // movement orders from the gate.
        if (!bot->GetSession() || !bot->GetSession()->IsBot())
            return false;

        float const anchorX = state.targetPoi.GetPositionX();
        float const anchorY = state.targetPoi.GetPositionY();
        float const anchorZ = state.targetPoi.GetPositionZ();

        float targetX = opponent->GetPositionX();
        float targetY = opponent->GetPositionY();
        float targetZ = opponent->GetPositionZ();
        float const dx = targetX - anchorX;
        float const dy = targetY - anchorY;
        float const distFromAnchor = std::sqrt(dx * dx + dy * dy);
        float const leash = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;

        if (distFromAnchor > leash && distFromAnchor > 0.1f)
        {
            float const scale = leash / distFromAnchor;
            targetX = anchorX + dx * scale;
            targetY = anchorY + dy * scale;
            targetZ = anchorZ;
        }

        float const angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
        float const offset = frand(2.5f, 5.0f);
        targetX += std::cos(angle) * offset;
        targetY += std::sin(angle) * offset;
        ClampTravelZ(bot, targetX, targetY, targetZ);
        PrepareForMovement(bot, state);
        RecordMove(bot, "duel-toward", targetX, targetY, targetZ, 0);
        bot->GetMotionMaster()->MovePoint(0, targetX, targetY, targetZ);
        return true;
    }

    Player* GetReservedDuelOpponent(Player* bot)
    {
        if (!bot)
            return nullptr;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        auto reserve = s_duelReservedBy.find(guidLow);
        if (reserve == s_duelReservedBy.end())
            return nullptr;

        uint32 const reservedAt = s_duelReserveMs[guidLow];
        if (!reservedAt || GetMSTimeDiffToNow(reservedAt) > kDuelReserveTimeoutMs)
        {
            ClearDuelReservation(guidLow);
            return nullptr;
        }

        Player* opponent = FindConnectedRosterBot(reserve->second);
        if (!opponent)
            ClearDuelReservation(guidLow);

        return opponent;
    }

    void ComputeDuelMeetPoint(Player* bot, Player* opponent, CitizenState const& state,
                              float& meetX, float& meetY, float& meetZ)
    {
        float const anchorX = state.targetPoi.GetPositionX();
        float const anchorY = state.targetPoi.GetPositionY();
        float const anchorZ = state.targetPoi.GetPositionZ();

        meetX = (bot->GetPositionX() + opponent->GetPositionX()) * 0.5f;
        meetY = (bot->GetPositionY() + opponent->GetPositionY()) * 0.5f;
        meetZ = (bot->GetPositionZ() + opponent->GetPositionZ()) * 0.5f;

        float const leash = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;
        float const dx = meetX - anchorX;
        float const dy = meetY - anchorY;
        float const distFromAnchor = std::sqrt(dx * dx + dy * dy);
        if (distFromAnchor > leash && distFromAnchor > 0.1f)
        {
            float const scale = leash / distFromAnchor;
            meetX = anchorX + dx * scale;
            meetY = anchorY + dy * scale;
            meetZ = anchorZ;
        }

        bot->UpdateAllowedPositionZ(meetX, meetY, meetZ);
    }

    void ComputeDuelStandPoint(Player* bot, Player* opponent, float meetX, float meetY,
                               float meetZ, float& standX, float& standY, float& standZ)
    {
        standX = meetX;
        standY = meetY;
        standZ = meetZ;
        if (!bot || !opponent)
            return;

        bool const botIsLowerGuid =
            bot->GetGUID().GetCounter() < opponent->GetGUID().GetCounter();
        float dx = botIsLowerGuid
            ? opponent->GetPositionX() - bot->GetPositionX()
            : bot->GetPositionX() - opponent->GetPositionX();
        float dy = botIsLowerGuid
            ? opponent->GetPositionY() - bot->GetPositionY()
            : bot->GetPositionY() - opponent->GetPositionY();
        if (std::fabs(dx) < 0.1f && std::fabs(dy) < 0.1f)
        {
            dx = 1.0f;
            dy = 0.0f;
        }

        float const angle = std::atan2(dy, dx) + static_cast<float>(M_PI) * 0.5f;
        float const side = botIsLowerGuid ? 1.0f : -1.0f;
        float const offset = IsRangedDueler(bot) ? 4.8f : 2.4f;

        standX += std::cos(angle) * offset * side;
        standY += std::sin(angle) * offset * side;
    }

    void ReserveDuelAtMeetingPoint(Player* bot, Player* opponent, CitizenState const& state)
    {
        if (!bot || !opponent)
            return;

        float meetX = state.targetPoi.GetPositionX();
        float meetY = state.targetPoi.GetPositionY();
        float meetZ = state.targetPoi.GetPositionZ();
        ComputeDuelMeetPoint(bot, opponent, state, meetX, meetY, meetZ);
        ReserveDuelPair(bot->GetGUID().GetCounter(), opponent->GetGUID().GetCounter(),
                        meetX, meetY, meetZ);
    }

    bool MoveToDuelMeeting(Player* bot, CitizenState& state, Player* opponent)
    {
        if (!bot || !opponent)
            return false;

        // Real players may be duel opponents, but only bot sessions take
        // movement orders from the gate.
        if (!bot->GetSession() || !bot->GetSession()->IsBot())
            return false;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        auto meetX = s_duelMeetX.find(guidLow);
        auto meetY = s_duelMeetY.find(guidLow);
        auto meetZ = s_duelMeetZ.find(guidLow);
        if (meetX == s_duelMeetX.end() || meetY == s_duelMeetY.end() ||
            meetZ == s_duelMeetZ.end())
            return MoveTowardDuelOpponent(bot, state, opponent);

        float x = meetX->second;
        float y = meetY->second;
        float z = meetZ->second;
        ComputeDuelStandPoint(bot, opponent, x, y, z, x, y, z);
        bot->UpdateAllowedPositionZ(x, y, z);

        if (bot->GetDistance(x, y, z) <= 1.6f)
            return false;

        PrepareForMovement(bot, state);
        RecordMove(bot, "duel-meeting", x, y, z, 0);
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        return true;
    }

    bool DriveReservedDuel(Player* bot, CitizenState& state, Player* opponent)
    {
        if (!bot || !opponent)
            return false;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!IsDuelReservationUsable(bot, opponent, state.role))
        {
            ClearDuelReservation(guidLow);
            return false;
        }

        float const meetX = s_duelMeetX[guidLow];
        float const meetY = s_duelMeetY[guidLow];
        float const meetZ = s_duelMeetZ[guidLow];
        float botStandX = meetX;
        float botStandY = meetY;
        float botStandZ = meetZ;
        float opponentStandX = meetX;
        float opponentStandY = meetY;
        float opponentStandZ = meetZ;
        ComputeDuelStandPoint(bot, opponent, meetX, meetY, meetZ,
                              botStandX, botStandY, botStandZ);
        ComputeDuelStandPoint(opponent, bot, meetX, meetY, meetZ,
                              opponentStandX, opponentStandY, opponentStandZ);
        bot->UpdateAllowedPositionZ(botStandX, botStandY, botStandZ);
        opponent->UpdateAllowedPositionZ(opponentStandX, opponentStandY, opponentStandZ);

        uint32 const reservedAt = s_duelReserveMs[guidLow];
        uint32 const reserveAge = reservedAt ? GetMSTimeDiffToNow(reservedAt) : 0;
        bool const botReady = bot->GetDistance(botStandX, botStandY, botStandZ) <= 3.5f;
        bool const opponentReady =
            opponent->GetDistance(opponentStandX, opponentStandY, opponentStandZ) <= 3.5f;
        bool const inDuelRange = bot->IsWithinDistInMap(opponent, 11.5f);
        bool const tooClose = bot->IsWithinDistInMap(opponent, 3.0f);
        bool const hasLos = bot->IsWithinLOSInMap(opponent);
        bool const stillSeparating =
            reserveAge < 3 * IN_MILLISECONDS && (tooClose || (!botReady && !opponentReady));

        if (!inDuelRange || !hasLos || stillSeparating)
        {
            if (Throttle(s_directMoveMs, guidLow, 1 * IN_MILLISECONDS))
            {
                MoveToDuelMeeting(bot, state, opponent);
                MoveToDuelMeeting(opponent, state, bot);
            }
            return true;
        }

        if (!bot->HasInArc(CAST_ANGLE_IN_FRONT, opponent, sPlayerbotAIConfig.sightDistance))
            bot->SetFacingTo(bot->GetAngle(opponent));

        if (s_duelProposer[guidLow] != guidLow)
            return true;

        SpellCastResult const result = bot->CastSpell(opponent, 7266, false);
        if (result == SPELL_CAST_OK)
        {
            uint32 const opponentGuid = opponent->GetGUID().GetCounter();
            RecordDuelPair(guidLow, opponentGuid);
            s_directDuelMs[opponentGuid] = getMSTime();
        }
        else if (s_duelBlockedLogged.insert(guidLow).second)
            CbLog::Warn("city dueler {} (guid {}) failed to challenge reserved opponent {} (guid {}): spell result {}",
                        bot->GetName(), guidLow, opponent->GetName(),
                        opponent->GetGUID().GetCounter(), static_cast<uint32>(result));

        return true;
    }

    CityPoi const* PickDirectRoamPoi(CitizenState& state, uint32 guidLow)
    {
        uint32 const phase = getMSTime() / ((28 + (guidLow % 19)) * IN_MILLISECONDS);

        if (state.homeLocationKey == "Goldshire")
        {
            static uint32 const route[] = {
                401, // inn
                410, // blacksmith road
                416, // pond
                411, // inn yard
                412, // west road
                414, // upstairs spectator
                413, // east road
                402, // tavern
                417, // pond
                415, // upstairs hall
                403, // center
                407  // center edge
            };

            uint32& step = s_goldshireRouteStep[guidLow];
            uint32 const count = sizeof(route) / sizeof(route[0]);
            uint32 const start = (guidLow + step++) % count;
            for (uint32 i = 0; i < count; ++i)
            {
                uint32 const poiId = route[(start + i) % count];
                if (poiId == state.targetPoiId)
                    continue;

                if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(poiId))
                    return poi;
            }

            return CityPoiRegistry::Instance().GetPoi(route[(guidLow + phase) % count]);
        }

        auto isHubPoi = [](CityPoi const* poi)
        {
            return poi && (IsAlternatingHubPoi(poi) ||
                           poi->poiType == CITY_POI_INN ||
                           poi->poiType == CITY_POI_AUCTION_HOUSE ||
                           poi->poiType == CITY_POI_BANK);
        };

        if (UsesAlternatingHubRoute(state.homeLocationKey))
        {
            bool lastWasHub = true;
            auto const remembered = s_stormwindDirectLastIssuedWasTrade.find(guidLow);
            if (remembered != s_stormwindDirectLastIssuedWasTrade.end())
                lastWasHub = remembered->second;
            else if (state.stormwindRouteSeen)
                lastWasHub = state.stormwindLastIssuedWasTrade;

            uint32 const seed = guidLow * 2654435761u + phase * 1013904223u;
            if (lastWasHub)
            {
                for (uint32 attempt = 0; attempt < 12; ++attempt)
                {
                    if (CityPoi const* poi = CityPoiRegistry::Instance().PickRoamPoi(
                            state.homeLocationKey, seed + attempt * 374761393u, state.targetPoiId,
                            false, HubExcludeDistrict(state.homeLocationKey)))
                    {
                        if (!IsAlternatingOutsidePoi(poi))
                            continue;

                        state.lastDirectRoamWasHub = false;
                        state.stormwindRouteSeen = true;
                        state.stormwindLastIssuedWasTrade = false;
                        s_stormwindDirectLastIssuedWasTrade[guidLow] = false;
                        return poi;
                    }
                }

                return nullptr;
            }

            CityPoiType const hubTypes[] = {
                CITY_POI_AUCTION_HOUSE,
                CITY_POI_BANK,
                CITY_POI_INN
            };
            uint32 const start = (guidLow + phase) % (sizeof(hubTypes) / sizeof(hubTypes[0]));
            for (uint32 i = 0; i < sizeof(hubTypes) / sizeof(hubTypes[0]); ++i)
            {
                if (CityPoi const* poi = CityPoiRegistry::Instance().PickPoi(
                        state.homeLocationKey,
                        hubTypes[(start + i) % (sizeof(hubTypes) / sizeof(hubTypes[0]))],
                        state.targetPoiId))
                {
                    if (!IsAlternatingHubPoi(poi))
                        continue;

                    state.lastDirectRoamWasHub = true;
                    state.stormwindRouteSeen = true;
                    state.stormwindLastIssuedWasTrade = true;
                    s_stormwindDirectLastIssuedWasTrade[guidLow] = true;
                    return poi;
                }
            }

            return nullptr;
        }

        if (!state.homeLocationKey.empty() &&
            state.homeLocationKey != "StormwindGate" &&
            state.homeLocationKey != "OrgrimmarGate")
        {
            CityPoi const* previous = CityPoiRegistry::Instance().GetPoi(state.targetPoiId);
            bool const previousWasHub =
                isHubPoi(previous) || state.lastDirectRoamWasHub ||
                (UsesAlternatingHubRoute(state.homeLocationKey) && !previous);
            if (!previousWasHub)
            {
                CityPoiType const hubTypes[] = {
                    CITY_POI_AUCTION_HOUSE,
                    CITY_POI_BANK,
                    CITY_POI_INN
                };
                uint32 const start = (guidLow + phase) % (sizeof(hubTypes) / sizeof(hubTypes[0]));
                for (uint32 i = 0; i < sizeof(hubTypes) / sizeof(hubTypes[0]); ++i)
                {
                    CityPoiType const type = hubTypes[(start + i) % (sizeof(hubTypes) / sizeof(hubTypes[0]))];
                    if (CityPoi const* poi = CityPoiRegistry::Instance().PickPoi(
                            state.homeLocationKey, type, state.targetPoiId))
                    {
                        state.lastDirectRoamWasHub = true;
                        return poi;
                    }
                }
            }

            uint32 const seed = guidLow * 2654435761u + phase * 1013904223u;
            std::string const excludeDistrict =
                previousWasHub
                    ? (previous ? previous->district : HubExcludeDistrict(state.homeLocationKey))
                    : "";
            for (uint32 attempt = 0; attempt < 8; ++attempt)
            {
                if (CityPoi const* poi = CityPoiRegistry::Instance().PickRoamPoi(
                        state.homeLocationKey, seed + attempt * 374761393u, state.targetPoiId,
                        false, excludeDistrict))
                {
                    state.lastDirectRoamWasHub = false;
                    return poi;
                }
            }

            if (UsesAlternatingHubRoute(state.homeLocationKey))
                return nullptr;

            if (previousWasHub)
            {
                for (uint32 attempt = 0; attempt < 8; ++attempt)
                {
                    if (CityPoi const* poi = CityPoiRegistry::Instance().PickRoamPoi(
                            state.homeLocationKey, seed + attempt * 2246822519u, state.targetPoiId,
                            true))
                    {
                        state.lastDirectRoamWasHub = isHubPoi(poi);
                        return poi;
                    }
                }
            }
        }

        uint32 const seed = guidLow * 2654435761u + phase * 1013904223u;
        if (CityPoi const* poi = CityPoiRegistry::Instance().PickRoamPoi(
                state.homeLocationKey, seed, state.targetPoiId))
        {
            state.lastDirectRoamWasHub = isHubPoi(poi);
            return poi;
        }

        if (state.targetPoiId)
        {
            CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId);
            state.lastDirectRoamWasHub = isHubPoi(poi);
            return poi;
        }

        return nullptr;
    }

    bool MoveTowardPoi(Player* bot, CitizenState& state, CityPoi const* poi, float spread)
    {
        if (!bot || !poi)
            return false;

        WorldPosition pos = CityPoiRegistry::Instance().GetWorldPosition(*poi);
        float const angle = frand(0.0f, 2.0f * static_cast<float>(M_PI));
        float const effectiveSpread =
            (poi->locationKey == "Goldshire" && poi->z > 60.0f)
                ? std::min(spread, 2.5f)
                : spread;
        float const distance = frand(1.5f, effectiveSpread);
        float x = pos.GetPositionX() + std::cos(angle) * distance;
        float y = pos.GetPositionY() + std::sin(angle) * distance;
        float z = pos.GetPositionZ();

        if (poi->poiType != CITY_POI_INN_BED)
            ClampTravelZ(bot, x, y, z);

        // Never hand MovePoint a destination without a real walkable path:
        // PointMovementGenerator falls back to a straight-line spline, which is
        // the "citizens flying across Orgrimmar / clipping through the floor"
        // class (seen live 2026-08-16, the first night movement was un-frozen).
        // In cities the JITTERED destination frequently resolves onto a roof or
        // inside a building (18y spread + vmap height), which has no walkable
        // route — a plain refusal froze whole cities (0/45 movers in Orgrimmar,
        // measured live). So: jittered point first, exact curated POI point as
        // the fallback, refuse only when BOTH fail — with a throttled log of
        // the path types so the next tuning round has evidence.
        uint32 finalPathType = 0;
        {
            auto walkable = [&](float px, float py, float pz)
            {
                PathGenerator path(bot);
                if (!path.CalculatePath(px, py, pz, false))
                    return std::make_pair(false, uint32(PATHFIND_NOPATH));
                uint32 const type = path.GetPathType();
                bool ok =
                    !(type & (PATHFIND_NOPATH | PATHFIND_SHORTCUT | PATHFIND_NOT_USING_PATH));
                // INCOMPLETE is normal on a city mesh (banning it froze
                // Orgrimmar, round 9) BUT its straight tail is how Wesrow
                // walked off the EDGE of Dalaran (forensics: z 655 -> 638 ->
                // 585 -> 550, "last order poi ... pathType 4"). Accept an
                // incomplete path only when the mesh part ends within 15y of
                // the destination -- a short tail on flat floor is a step, a
                // long one over a floating city's rim is a fall.
                if (ok && (type & PATHFIND_INCOMPLETE))
                {
                    G3D::Vector3 const& end = path.GetActualEndPosition();
                    float const tx = end.x - px, ty = end.y - py, tz = end.z - pz;
                    if (tx * tx + ty * ty + tz * tz > 15.0f * 15.0f)
                        ok = false;
                }
                if (ok && PathTouchesWater(bot, path))
                    ok = false;
                return std::make_pair(ok, type);
            };

            // THE SINK (forensics 2026-08-18): the jittered destination's z
            // resolves through ClampTravelZ onto the WATER PLANE under the
            // district (Stormwind trade floor 95 -> canal water 59.5; Orgrimmar
            // 10 -> -2.8), and there IS a navmesh path down to it, so the
            // walkability check passes and the bot walks itself under the
            // city. Guard: a jitter whose resolved z is a level away from the
            // curated POI z is not this floor -- treat it as unwalkable so the
            // exact-POI fallback (curated z) is used instead. This is a REFUSAL
            // of the point, never a rewrite of z (round 4's rewrite broke the
            // poly lookup and froze everything).
            bool jitterOk = false;
            uint32 jitterType = PATHFIND_NOPATH;
            {
                // Same discriminator as the leg guard: the jitter is on the
                // water plane iff its resolved z is below every home POI
                // within 60y. (The earlier "within 6y of the curated POI z"
                // test also refused legitimate jitters onto Orgrimmar's
                // neighbouring terraces.)
                float minZ = 0.0f;
                bool const nearPois = CityPoiRegistry::Instance().LowestPoiZWithin(
                    state.homeLocationKey, x, y, 60.0f, &minZ);
                bool const onWaterPlane =
                    nearPois ? (z < minZ - 6.0f) : (pos.GetPositionZ() - z > 12.0f);
                if (!onWaterPlane)
                    std::tie(jitterOk, jitterType) = walkable(x, y, z);
            }
            finalPathType = jitterType;
            if (!jitterOk)
            {
                auto [exactOk, exactType] =
                    walkable(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                float const exactDist = bot->GetDistance(
                    pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                if (!exactOk && exactDist >= 25.0f)
                {
                    // Long route with no full path (live: 585 refusals/boot —
                    // cross-district routes routinely exceed what one
                    // CalculatePath returns). Before refusing, try a LEG.
                    //
                    // FIRST the mesh-guided leg: ask the pathfinder for the
                    // route to the exact POI and, when it comes back
                    // INCOMPLETE with a mesh part worth walking, go to the
                    // point where the mesh part ENDS. That point is on the
                    // navmesh by construction, follows ramps and bridges, and
                    // can never resolve through a floor -- unlike a geometric
                    // 30y point, which from a plateau edge (Orgrimmar's Valley
                    // of Honor, z 43 over a z 9 gorge) resolves 30y down every
                    // time and was refused on all 5 headings (69/100 leg
                    // refusals per boot, type 0 = never path-tested).
                    {
                        PathGenerator guided(bot);
                        if (guided.CalculatePath(pos.GetPositionX(), pos.GetPositionY(),
                                                 pos.GetPositionZ(), false))
                        {
                            uint32 const gtype = guided.GetPathType();
                            G3D::Vector3 const& mend = guided.GetActualEndPosition();
                            float const gdx = mend.x - bot->GetPositionX();
                            float const gdy = mend.y - bot->GetPositionY();
                            float const gdist = std::sqrt(gdx * gdx + gdy * gdy);
                            if ((gtype & PATHFIND_INCOMPLETE) &&
                                !(gtype & (PATHFIND_NOPATH | PATHFIND_SHORTCUT | PATHFIND_NOT_USING_PATH)) &&
                                gdist >= 10.0f && !PathTouchesWater(bot, guided))
                            {
                                uint32 const legGuid = bot->GetGUID().GetCounter();
                                state.targetPoi = WorldPosition(
                                    poi->mapId, pos.GetPositionX(), pos.GetPositionY(),
                                    pos.GetPositionZ(), poi->orientation);
                                state.targetPoiId = poi->id;
                                state.atPoi = false;
                                PrepareForMovement(bot, state);
                                RecordMove(bot, "leg-mesh", mend.x, mend.y, mend.z, gtype);
                                bot->GetMotionMaster()->MovePoint(0, mend.x, mend.y, mend.z);
                                s_directMoveMs[legGuid] = getMSTime();
                                s_pathRefuseStreak.erase(legGuid);
                                return true;
                            }
                        }
                    }

                    // THEN the geometric fan, for spots where even the mesh
                    // part is too short to be worth walking.
                    {
                        float const bx = bot->GetPositionX();
                        float const by = bot->GetPositionY();
                        float const ddx = pos.GetPositionX() - bx;
                        float const ddy = pos.GetPositionY() - by;
                        float const norm = std::sqrt(ddx * ddx + ddy * ddy);
                        if (norm > 1.0f)
                        {
                            // Fan of leg headings: straight at the target
                            // first, then +/-35 and +/-70 degrees. In a dense
                            // city the direct 30y point is inside a wall most
                            // of the time (round 11: 4 legs taken per boot,
                            // Orgrimmar 7/74 movers); a wall is rarely on ALL
                            // five headings.
                            static float const kFan[] = { 0.0f, 0.61f, -0.61f, 1.22f, -1.22f };
                            float const baseAngle = std::atan2(ddy, ddx);
                            uint32 lastLegType = 0;
                            float lastLegDrop = 0.0f;
                            for (float const da : kFan)
                            {
                                float const a = baseAngle + da;
                                float lx = bx + std::cos(a) * 30.0f;
                                float ly = by + std::sin(a) * 30.0f;
                                float lz = bot->GetPositionZ();
                                ClampTravelZ(bot, lx, ly, lz);
                                // Water-plane guard: a leg whose resolved z is
                                // BELOW EVERY home POI within 60y is on the
                                // water table (forensics: leg orders to z=59.5
                                // in Stormwind). NB a plain "drop > 8y" test
                                // refused 64/85 Orgrimmar legs per boot: that
                                // city is a canyon and a 30y leg legitimately
                                // descends 8-20y between the Valley and the
                                // Drag. Below-all-POIs is the discriminator a
                                // canyon floor passes and a water plane fails.
                                lastLegDrop = bot->GetPositionZ() - lz;
                                {
                                    float minZ = 0.0f;
                                    bool const nearPois = CityPoiRegistry::Instance().LowestPoiZWithin(
                                        state.homeLocationKey, lx, ly, 60.0f, &minZ);
                                    // No POI within 60y (outskirts): fall back
                                    // to a plain drop test.
                                    bool const onWaterPlane =
                                        nearPois ? (lz < minZ - 6.0f) : (lastLegDrop > 12.0f);
                                    if (onWaterPlane)
                                        continue;
                                }
                                auto [legOk, legType] = walkable(lx, ly, lz);
                                lastLegType = legType;
                                if (!legOk)
                                    continue;

                                uint32 const legGuid = bot->GetGUID().GetCounter();
                                state.targetPoi = WorldPosition(
                                    poi->mapId, pos.GetPositionX(), pos.GetPositionY(),
                                    pos.GetPositionZ(), poi->orientation);
                                state.targetPoiId = poi->id;
                                state.atPoi = false;
                                PrepareForMovement(bot, state);
                                RecordMove(bot, "leg", lx, ly, lz, legType);
                                bot->GetMotionMaster()->MovePoint(0, lx, ly, lz);
                                s_directMoveMs[legGuid] = getMSTime();
                                s_pathRefuseStreak.erase(legGuid);
                                return true;
                            }
                            static std::unordered_map<uint32, uint32> s_legRefuseLogMs;
                            if (Throttle(s_legRefuseLogMs, bot->GetGUID().GetCounter(),
                                         60 * IN_MILLISECONDS))
                                CbLog::Info("all 5 legs refused for {} at ({:.1f},{:.1f},{:.1f}) toward poi {} "
                                            "(last leg type {}, last drop {:.1f})",
                                            bot->GetName(), bx, by, bot->GetPositionZ(), poi->id,
                                            lastLegType, lastLegDrop);
                        }
                    }

                    // No full path AND no walkable leg — a bot that keeps
                    // failing from the SAME spot is marooned somewhere
                    // pathfinding cannot leave (live 2026-08-17: bots on the
                    // Orgrimmar water table, 200-500y from their POI — outside
                    // every geometric rescue net). Three strikes and it
                    // re-stages through the fixed resolver.
                    uint32 const gl = bot->GetGUID().GetCounter();
                    if (++s_pathRefuseStreak[gl] >= 3)
                    {
                        s_pathRefuseStreak.erase(gl);
                        CbLog::Info("re-staging {} (guid {}): marooned at "
                                    "({:.1f},{:.1f},{:.1f}), no walkable path 3x "
                                    "(jitter type {}, exact type {}, dist {:.0f})",
                                    bot->GetName(), gl, bot->GetPositionX(),
                                    bot->GetPositionY(), bot->GetPositionZ(),
                                    jitterType, exactType, exactDist);
                        TeleportToHome(bot, state);
                        return false;
                    }
                    static std::unordered_map<uint32, uint32> s_pathRefuseLogMs;
                    if (Throttle(s_pathRefuseLogMs, gl, 60 * IN_MILLISECONDS))
                        CbLog::Info("no walkable path for {} at ({:.1f},{:.1f},{:.1f}) to poi {} "
                                    "(jitter type {}, exact type {}, dist {:.0f})",
                                    bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                                    bot->GetPositionZ(), poi->id, jitterType, exactType,
                                    exactDist);
                    return false;
                }
                // Exact POI point is walkable, or close enough (<25y) that a
                // short straight hop reads as normal walking on an imperfect
                // mesh — never as cross-city flight.
                finalPathType = exactType;
                x = pos.GetPositionX();
                y = pos.GetPositionY();
                z = pos.GetPositionZ();
            }
        }

        uint32 const guidLow = bot->GetGUID().GetCounter();
        bool const newDestination = state.targetPoiId != poi->id;
        state.targetPoi = WorldPosition(poi->mapId, x, y, z, poi->orientation);
        state.targetPoiId = poi->id;
        state.atPoi = false;

        if (newDestination)
            s_directMountDecisionPoiId[guidLow] = 0;

        MaybeMountForPoiTravel(bot, state);

        PrepareForMovement(bot, state);
        RecordMove(bot, "poi", x, y, z, finalPathType);
        bot->GetMotionMaster()->MovePoint(0, x, y, z);
        s_directMoveMs[guidLow] = getMSTime();
        s_pathRefuseStreak.erase(guidLow);
        return true;
    }

    void HealRestingDueler(Player* bot)
    {
        if (!bot || bot->duel || bot->IsInCombat())
            return;

        RestoreDuelerVitals(bot);
    }

    void ClearStaleRoamMovement(Player* bot, CitizenState& state)
    {
        if (!bot)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        uint32 const lastMoveMs = s_directMoveMs[guidLow];
        uint32 const timeoutMs = state.homeLocationKey == "Goldshire"
            ? kGoldshireRoamTravelTimeoutMs
            : kCityRoamTravelTimeoutMs;

        if (lastMoveMs && GetMSTimeDiffToNow(lastMoveMs) < timeoutMs)
            return;

        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        state.targetPoi = WorldPosition();
        state.targetPoiId = 0;
        state.atPoi = true;
        s_directMoveMs.erase(guidLow);
    }

    void DriveDancer(Player* bot, CitizenState& state)
    {
        EnsureDedicatedStagePosition(bot, state);
        if (!bot || bot->IsBeingTeleported())
            return;

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
        state.activity = CITIZEN_INN_DANCE;
        state.atPoi = true;

        Player* nearby = FindNearbyRealPlayer(bot, 1.5f);
        if (s_dancerLogged.insert(bot->GetGUID().GetCounter()).second)
            CbLog::Info("driving inn dancer {} (guid {}) at poi {}",
                        bot->GetName(), bot->GetGUID().GetCounter(), state.targetPoiId);

        uint32 const greetCooldownMs =
            std::max<uint32>(CbSettings::GetUInt("SayCooldownSec"), 30) * IN_MILLISECONDS;
        if (!nearby || !Throttle(s_dancerGreetMs, bot->GetGUID().GetCounter(), greetCooldownMs))
            return;

        bot->SetFacingTo(bot->GetAngle(nearby));
        static char const* greetings[] = {
            "Hey there, {}.",
            "Welcome upstairs, {}.",
            "Enjoying Goldshire, {}?",
            "Hi, {}."
        };
        bot->Say(Acore::StringFormat(greetings[urand(0, 3)], nearby->GetName()),
                 LANG_UNIVERSAL);
    }

    void DriveDueler(Player* bot, CitizenState& state)
    {
        if (!bot)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (RecoverDeadDueler(bot, state))
            return;

        AcceptPendingDuel(bot, state);
        EnsureDedicatedStagePosition(bot, state);

        if (bot->duel)
        {
            s_wasDueling.insert(guidLow);
            DriveActiveDuel(bot, state);
            return;
        }

        if (bot->IsInCombat())
        {
            s_wasDueling.insert(guidLow);
            if (Throttle(s_duelAttackMs, guidLow, 8 * IN_MILLISECONDS))
            {
                CbLog::Info("clearing city dueler {} (guid {}) from combat without duel",
                            bot->GetName(), guidLow);
                bot->CombatStopWithPets(true);
                bot->ClearInCombat();
                bot->StopMoving();
                bot->GetMotionMaster()->Clear();
                RestoreDuelerVitals(bot);
                s_duelCooldownMs[guidLow] = getMSTime();
                s_directMoveMs.erase(guidLow);
                ClearDuelTracking(guidLow);
            }
            return;
        }

        if (s_wasDueling.erase(guidLow) > 0)
        {
            s_duelCooldownMs[guidLow] = getMSTime();
            s_directDuelMs.erase(guidLow);
            s_directMoveMs.erase(guidLow);
            ClearDuelTracking(guidLow);
            MaybeDuelFlourish(bot);
        }

        if (bot->IsBeingTeleported())
            return;

        HealRestingDueler(bot);

        float const roam = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kGateDuelerRoamRadius
            : kGoldshireDuelerRoamRadius;
        float const returnDistance = state.role == CITIZEN_ROLE_GATE_DUELIST
            ? kDuelerReturnDistance
            : 12.0f;

        if (!IsAllowedDuelerDpsSpec(bot))
        {
            if (bot->IsInCombat())
                bot->CombatStopWithPets(true);
            MoveAroundTarget(bot, state, roam, 5 * IN_MILLISECONDS, returnDistance);
            return;
        }

        EnsureDuelerCombatForm(bot);

        if (IsDuelCooldownActive(bot))
        {
            if (!MoveAroundTarget(bot, state, roam, 3 * IN_MILLISECONDS, returnDistance))
                MaybeDuelFlourish(bot);
            return;
        }

        if (Player* reserved = GetReservedDuelOpponent(bot))
        {
            if (DriveReservedDuel(bot, state, reserved))
                return;
        }

        if (!Throttle(s_directDuelMs, guidLow, 2 * IN_MILLISECONDS))
            return;

        Player* opponent = FindDirectDuelOpponent(bot, state);
        if (!opponent)
        {
            if (!MoveAroundTarget(bot, state, roam * 0.35f, 5 * IN_MILLISECONDS, returnDistance))
                MaybeDuelFlourish(bot);
            return;
        }

        ReserveDuelAtMeetingPoint(bot, opponent, state);
        if (s_duelLogged.insert(guidLow).second)
            CbLog::Info("city dueler {} (guid {}) reserved {} (guid {}) for duel near {}",
                        bot->GetName(), guidLow, opponent->GetName(),
                        opponent->GetGUID().GetCounter(), state.homeLocationKey);
        DriveReservedDuel(bot, state, opponent);
        return;
    }

    // Nearest home POIs first, the first one (other than the current target)
    // with a walkable path wins. At most 5 path queries per call.
    CityPoi const* PickReachableLocalPoi(Player* bot, CitizenState& state, uint32 guidLow)
    {
        if (!bot || state.homeLocationKey.empty())
            return nullptr;

        std::vector<std::pair<float, CityPoi const*>> ranked;
        for (CityPoi const& p : CityPoiRegistry::Instance().All())
        {
            CityPoi const* poi = &p;
            if (poi->locationKey != state.homeLocationKey || poi->id == state.targetPoiId)
                continue;
            if (poi->poiType == CITY_POI_INN_BED || poi->poiType == CITY_POI_DOCK)
                continue;
            float const dx = poi->x - bot->GetPositionX();
            float const dy = poi->y - bot->GetPositionY();
            float const d2 = dx * dx + dy * dy;
            if (d2 < 15.0f * 15.0f)
                continue;   // already here
            ranked.emplace_back(d2, poi);
        }
        std::sort(ranked.begin(), ranked.end(),
                  [](auto const& a, auto const& b) { return a.first < b.first; });

        // Rotate the start by bot so a district's citizens do not all pick
        // the same nearest POI.
        std::size_t const n = std::min<std::size_t>(ranked.size(), 5);
        for (std::size_t k = 0; k < n; ++k)
        {
            CityPoi const* poi = ranked[(k + guidLow) % n].second;
            if (HasCompleteWalkablePath(bot, poi->x, poi->y, poi->z))
                return poi;
        }
        return nullptr;
    }

    void DriveAmbience(Player* bot, CitizenState& state)
    {
        if (!bot || bot->duel || bot->IsInCombat() ||
            state.role == CITIZEN_ROLE_INN_DANCER || IsDuelerRole(state.role))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        uint32 const moveInterval = (state.homeLocationKey == "Goldshire"
            ? (5 + ((guidLow * 3) % 9))
            : (14 + ((guidLow * 7) % 23))) * IN_MILLISECONDS;

        // One line per bot per boot: proves the ambience driver reaches this
        // bot at all (the frozen-city incident had no evidence either way).
        static std::unordered_set<uint32> s_ambienceLogged;
        if (s_ambienceLogged.insert(guidLow).second)
            CbLog::Info("driving ambience for {} (guid {}) home {} target-poi {}",
                        bot->GetName(), guidLow, state.homeLocationKey, state.targetPoiId);

        // Movement stall diagnostic: a MovePoint this driver issued should
        // have moved the bot within a few seconds. If the bot is provably
        // still at the issue position 10s later, something outside this module
        // is cancelling bot movement — say so once per episode.
        static std::unordered_map<uint32, uint32> s_moveIssuedMs;
        static std::unordered_map<uint32, std::pair<float, float>> s_moveIssuedPos;
        static std::unordered_set<uint32> s_moveStallWarned;
        auto recordMoveIssued = [&]()
        {
            s_moveIssuedMs[guidLow] = getMSTime();
            s_moveIssuedPos[guidLow] = { bot->GetPositionX(), bot->GetPositionY() };
        };
        {
            auto issued = s_moveIssuedMs.find(guidLow);
            if (issued != s_moveIssuedMs.end())
            {
                float const dx = bot->GetPositionX() - s_moveIssuedPos[guidLow].first;
                float const dy = bot->GetPositionY() - s_moveIssuedPos[guidLow].second;
                if (dx * dx + dy * dy > 1.0f)
                {
                    s_moveIssuedMs.erase(guidLow);
                    s_moveIssuedPos.erase(guidLow);
                    s_moveStallWarned.erase(guidLow);
                }
                else if (GetMSTimeDiffToNow(issued->second) > 10 * IN_MILLISECONDS &&
                         s_moveStallWarned.insert(guidLow).second)
                    CbLog::Warn("{} (guid {}) has not moved {}ms after MovePoint at "
                                "({:.1f},{:.1f}) -- movement is being blocked outside "
                                "mod-city-bots",
                                bot->GetName(), guidLow, GetMSTimeDiffToNow(issued->second),
                                bot->GetPositionX(), bot->GetPositionY());
            }
        }

        bool skipMove = false;
        if (state.targetPoi.IsValid() && !state.atPoi)
        {
            if (bot->IsWithinDist3d(state.targetPoi.GetPositionX(),
                                    state.targetPoi.GetPositionY(),
                                    state.targetPoi.GetPositionZ(), 4.0f))
            {
                state.atPoi = true;
                s_directMountDecisionPoiId[guidLow] = 0;
                s_directMountAttemptMs[guidLow] = 0;
                DismountIfMounted(bot);
                s_directMoveMs[guidLow] = getMSTime();
            }
            else if (bot->isMoving())
            {
                ClearStaleRoamMovement(bot, state);
                skipMove = state.targetPoi.IsValid() && !state.atPoi && bot->isMoving();
            }
        }

        bool const emoteHold =
            !bot->isMoving() && state.lastEmoteMs &&
            GetMSTimeDiffToNow(state.lastEmoteMs) < 8 * IN_MILLISECONDS;

        // Ambience decision trace (Debug, LogLevel 3): one line per bot per
        // 2 min showing every gate the move must pass. Added because 65/75
        // Orgrimmar citizens stood idle for whole boots with NOTHING in the
        // log -- neither a move nor a refusal.
        {
            static std::unordered_map<uint32, uint32> s_ambTraceMs;
            if (CbLog::ShouldLog(CbLog::DebugLevel) &&
                Throttle(s_ambTraceMs, guidLow, 120 * IN_MILLISECONDS))
            {
                uint32 const last = s_directMoveMs[guidLow];
                CbLog::Debug("AMB {} (guid {}) at ({:.1f},{:.1f},{:.1f}) home {} target {} atPoi={} "
                             "moving={} skipMove={} emoteHold={} sinceMove={}ms interval={}ms "
                             "mounted={} tele={} combat={} duel={}",
                             bot->GetName(), guidLow, bot->GetPositionX(), bot->GetPositionY(),
                             bot->GetPositionZ(), state.homeLocationKey, state.targetPoiId,
                             state.atPoi ? 1 : 0, bot->isMoving() ? 1 : 0, skipMove ? 1 : 0,
                             emoteHold ? 1 : 0, last ? GetMSTimeDiffToNow(last) : 0, moveInterval,
                             bot->HasMountedAura() ? 1 : 0, bot->IsBeingTeleported() ? 1 : 0,
                             bot->IsInCombat() ? 1 : 0, bot->duel ? 1 : 0);
            }
        }

        if (!skipMove && !emoteHold && Throttle(s_directMoveMs, guidLow, moveInterval))
        {
            if (CityPoi const* poi = PickDirectRoamPoi(state, guidLow))
            {
                if (MoveTowardPoi(bot, state, poi, state.homeLocationKey == "Goldshire" ? 9.0f : 18.0f))
                {
                    recordMoveIssued();
                    return;
                }

                // The route design's destination is unreachable from here
                // (Orgrimmar's hub<->outer-district legs are 300-500y across
                // a mesh that cannot path them: 65/75 citizens idle, round
                // 14). Instead of standing, roam LOCALLY: any POI of the home
                // city that IS reachable, nearest first, so the citizen still
                // walks the district it is in.
                CityPoi const* local = PickReachableLocalPoi(bot, state, guidLow);
                if (local && MoveTowardPoi(bot, state, local, 12.0f))
                {
                    recordMoveIssued();
                    return;
                }
                static std::unordered_map<uint32, uint32> s_roamFailLogMs;
                if (Throttle(s_roamFailLogMs, guidLow, 120 * IN_MILLISECONDS))
                    CbLog::Info("roam stalled for {} at ({:.1f},{:.1f},{:.1f}): route poi {} refused, "
                                "local poi {} {}",
                                bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                                bot->GetPositionZ(), poi->id, local ? local->id : 0,
                                local ? "refused too" : "none reachable");
            }
            else
            {
                static std::unordered_map<uint32, uint32> s_roamNullLogMs;
                if (Throttle(s_roamNullLogMs, guidLow, 120 * IN_MILLISECONDS))
                    CbLog::Info("roam pick returned nothing for {} at ({:.1f},{:.1f},{:.1f}) home {} "
                                "target {}",
                                bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                                bot->GetPositionZ(), state.homeLocationKey, state.targetPoiId);
            }

            if (MoveAroundTarget(bot, state, 10.0f, 1, 35.0f))
            {
                recordMoveIssued();
                return;
            }
        }
        else if (state.homeLocationKey == "Goldshire" && !skipMove && !emoteHold && state.atPoi &&
                 !bot->isMoving() &&
                 Throttle(s_directMoveMs, guidLow + 500000u,
                          (8 + ((guidLow * 5) % 11)) * IN_MILLISECONDS))
        {
            if (ShuffleAroundCurrentPoi(bot, state, 7.0f, 28.0f))
                return;
        }

        if (bot->HasMountedAura() || !state.atPoi || bot->isMoving())
            return;

        uint32 const emoteInterval = (6 + (guidLow % 8)) * IN_MILLISECONDS;
        if (!Throttle(s_directEmoteMs, guidLow, emoteInterval))
            return;

        if (state.sitting)
        {
            bot->SetStandState(UNIT_STAND_STATE_STAND);
            state.sitting = false;
        }

        state.lastEmoteMs = getMSTime();
        switch ((guidLow + (getMSTime() / (15 * IN_MILLISECONDS))) % 8)
        {
            case 0:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                break;
            case 1:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                break;
            case 2:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                break;
            case 3:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                break;
            case 4:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);
                break;
            case 5:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                break;
            default:
                bot->HandleEmoteCommand(EMOTE_ONESHOT_TALK);
                break;
        }
    }

    void DriveFisher(Player* bot, CitizenState& state)
    {
        if (!bot || bot->duel || bot->IsInCombat())
            return;

        EnsureDedicatedStagePosition(bot, state);

        if (!state.targetPoi.IsValid())
            return;

        if (bot->IsBeingTeleported())
            return;

        if (!bot->IsWithinDist3d(state.targetPoi.GetPositionX(),
                                 state.targetPoi.GetPositionY(),
                                 state.targetPoi.GetPositionZ(), 5.0f))
        {
            if (Throttle(s_directMoveMs, bot->GetGUID().GetCounter(), 3 * IN_MILLISECONDS))
            {
                if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId))
                    MoveTowardPoi(bot, state, poi, 2.5f);
            }
            return;
        }

        DismountIfMounted(bot);

        if (state.sitting)
        {
            bot->SetStandState(UNIT_STAND_STATE_STAND);
            state.sitting = false;
        }

        uint32 constexpr fishingSpell = 7620;
        if (!bot->HasSpell(fishingSpell))
            bot->learnSpell(fishingSpell, false);

        uint32 constexpr fishingPole = 6256;
        EquipItemReplacingSlot(bot, EQUIPMENT_SLOT_MAINHAND, fishingPole);

        if (bot->IsNonMeleeSpellCast(false))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if (!Throttle(s_directFishMs, guidLow, 17 * IN_MILLISECONDS))
            return;

        bot->SetFacingTo(state.targetPoi.GetOrientation());
        bot->CastSpell(bot, fishingSpell, false);
    }

    // ------------------------------------------------------------------
    // Crowd scenes, direct-drive port. The action-layer CitizenCrowdAction
    // never runs on this playerbots fork (the AI state pointer is null for
    // every citizen), so the gate drives the same design from
    // s_directRosterStates. Partner state resolves through the attached AI
    // state when the fork provides one, else the direct roster state — so the
    // paired chatter works on both arms.
    // ------------------------------------------------------------------

    CitizenState* DirectRosterStateFor(uint32 guidLow)
    {
        auto it = s_directRosterStates.find(guidLow);
        return it != s_directRosterStates.end() ? &it->second : nullptr;
    }

    CitizenState* AnyCitizenStateFor(Player* player)
    {
        if (!player)
            return nullptr;
        if (CitizenState* attached = CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(player)))
            return attached;
        return DirectRosterStateFor(player->GetGUID().GetCounter());
    }

    void CrowdClearAfkDirect(Player* bot)
    {
        if (bot->isAFK())
            bot->ToggleAFK();
    }

    bool CrowdSceneAwakeDirect(Player* bot)
    {
        if (!CbSettings::GetBool("EnableCrowdScenes"))
            return false;
        return FindNearbyRealPlayer(bot,
            static_cast<float>(CbSettings::GetUInt("CrowdPlayerRadius"))) != nullptr;
    }

    // Release a conversation claim on BOTH sides. Safe in any state; only
    // clears the partner's side if it still points back at this bot.
    void CrowdReleaseChatDirect(Player* bot, CitizenState& state)
    {
        if (!state.chatPartner.IsEmpty())
        {
            if (Player* partner = ObjectAccessor::FindConnectedPlayer(state.chatPartner))
                if (CitizenState* ps = AnyCitizenStateFor(partner))
                    if (ps->chatPartner == bot->GetGUID())
                    {
                        ps->chatPartner.Clear();
                        ps->chatInitiator = false;
                        ps->chatDialogueId = -1;
                        ps->chatLineIdx = 0;
                        ps->chatDeadlineMs = 0;
                        ps->lastCrowdChatMs = getMSTime();
                    }
        }
        state.chatPartner.Clear();
        state.chatInitiator = false;
        state.chatDialogueId = -1;
        state.chatLineIdx = 0;
        state.chatDeadlineMs = 0;
        state.lastCrowdChatMs = getMSTime();
    }

    Player* CrowdFindChatPartnerDirect(Player* bot)
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
            CitizenState* ps = AnyCitizenStateFor(player);
            if (!ps || ps->role != CITIZEN_ROLE_CROWD || ps->crowdPhase != CROWD_LINGER ||
                !ps->chatPartner.IsEmpty() || ps->activity != CITIZEN_CROWD_SOCIAL)
                continue;
            if (bot->GetDistance(player) > 15.0f)
                continue;
            return player;
        }
        return nullptr;
    }

    void CrowdBeginChatDirect(Player* bot, Player* partner, CitizenState& state, uint32 now)
    {
        CitizenState* ps = AnyCitizenStateFor(partner);
        if (!ps)
            return;

        // Pool by the initiator's home POI type; PLAZA pool is the generic fallback.
        CityPoiType pool = CITY_POI_PLAZA;
        if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.crowdHomePoiId))
            if (poi->poiType == CITY_POI_BANK || poi->poiType == CITY_POI_AUCTION_HOUSE)
                pool = poi->poiType;

        std::vector<int16> candidates;
        for (int16 i = 0; i < CrowdDialogues::kCrowdDialogueCount; ++i)
            if (CrowdDialogues::kCrowdDialogues[i].pool == pool ||
                CrowdDialogues::kCrowdDialogues[i].pool == CITY_POI_PLAZA)
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

    void DriveCrowd(Player* bot, CitizenState& state)
    {
        if (!bot || bot->duel || bot->IsInCombat() || bot->IsBeingTeleported())
            return;

        uint32 const now = getMSTime();
        uint32 const guidLow = bot->GetGUID().GetCounter();

        static std::unordered_set<uint32> s_crowdLogged;
        if (s_crowdLogged.insert(guidLow).second)
            CbLog::Info("driving crowd citizen {} (guid {}) at poi {}",
                        bot->GetName(), guidLow, state.targetPoiId);

        if (state.sitting)
        {
            bot->SetStandState(UNIT_STAND_STATE_STAND);
            state.sitting = false;
        }

        // Stale claim, EITHER side: full release so no claim outlives its
        // deadline (runs even while the scene is asleep).
        if (!state.chatPartner.IsEmpty() && now >= state.chatDeadlineMs)
            CrowdReleaseChatDirect(bot, state);

        if (!state.crowdHomePoiId)
            state.crowdHomePoiId = state.targetPoiId;

        // No real player near: stand cheaply, clear any leftover AFK.
        if (!CrowdSceneAwakeDirect(bot))
        {
            CrowdClearAfkDirect(bot);
            state.crowdPhase = CROWD_LINGER;
            state.crowdPhaseEndMs = 0;
            return;
        }

        // Arrival check while traveling between the pair POIs.
        if (state.targetPoi.IsValid() && !state.atPoi)
        {
            if (bot->IsWithinDist3d(state.targetPoi.GetPositionX(),
                                    state.targetPoi.GetPositionY(),
                                    state.targetPoi.GetPositionZ(), 4.0f))
            {
                state.atPoi = true;
                DismountIfMounted(bot);
            }
            else if (bot->isMoving())
                return;
        }

        switch (state.crowdPhase)
        {
            case CROWD_AFK:
                if (now < state.crowdPhaseEndMs)
                    return;                     // stay perfectly still
                CrowdClearAfkDirect(bot);
                state.crowdPhase = CROWD_LINGER;
                state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
                return;

            case CROWD_WANDER_OUT:
                if (now < state.crowdPhaseEndMs)
                    return;                     // dwelling at the partner POI
                if (CityPoi const* home = CityPoiRegistry::Instance().GetPoi(state.crowdHomePoiId))
                    MoveTowardPoi(bot, state, home, 8.0f);
                state.crowdPhase = CROWD_WANDER_BACK;
                state.crowdPhaseEndMs = now + 30 * IN_MILLISECONDS;
                return;

            case CROWD_WANDER_BACK:
                if (bot->isMoving() && now < state.crowdPhaseEndMs)
                    return;
                state.crowdPhase = CROWD_LINGER;
                state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
                return;

            case CROWD_LINGER:
            default:
                break;
        }

        // --- paired chatter (initiator drives both sides) ---
        if (CbSettings::GetBool("EnableSay"))
        {
            // Ongoing conversation I initiated: deliver the next line when due.
            if (state.chatInitiator && state.chatDialogueId >= 0 &&
                state.chatDialogueId < CrowdDialogues::kCrowdDialogueCount)
            {
                if (now >= state.chatDeadlineMs)
                    CrowdReleaseChatDirect(bot, state);
                else
                {
                    CrowdDialogues::CrowdDialogue const& d =
                        CrowdDialogues::kCrowdDialogues[state.chatDialogueId];
                    if (state.chatLineIdx < d.lineCount &&
                        now >= state.lastSayMs + urand(3000, 6000))
                    {
                        bool const partnerLine = (state.chatLineIdx % 2) == 1;
                        Player* speaker = bot;
                        if (partnerLine)
                        {
                            speaker = ObjectAccessor::FindConnectedPlayer(state.chatPartner);
                            if (!speaker || !speaker->IsInWorld() ||
                                bot->GetDistance(speaker) > 20.0f)
                            {
                                CrowdReleaseChatDirect(bot, state);
                                return;
                            }
                        }
                        speaker->Say(d.lines[state.chatLineIdx], LANG_UNIVERSAL);
                        state.lastSayMs = now;
                        ++state.chatLineIdx;
                        if (state.chatLineIdx >= d.lineCount)
                            CrowdReleaseChatDirect(bot, state);
                    }
                }
            }
            // Idle and off cooldown: try to start one.
            else if (state.chatPartner.IsEmpty() &&
                     GetMSTimeDiffToNow(state.lastCrowdChatMs) >
                         CbSettings::GetUInt("CrowdChatCooldownSec") * IN_MILLISECONDS &&
                     roll_chance_i(25))
            {
                if (Player* partner = CrowdFindChatPartnerDirect(bot))
                    CrowdBeginChatDirect(bot, partner, state, now);
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
            return;
        }

        // Linger expired, but hold LINGER while a conversation claim is active —
        // claims are deadline-bounded by the release at the top.
        if (!state.chatPartner.IsEmpty())
            return;

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
                afkMaxSec = afkMinSec;
            state.crowdPhaseEndMs = now + urand(afkMinSec, afkMaxSec) * IN_MILLISECONDS;
        }
        else if (roll <= 35)
        {
            if (uint32 partnerPoiId = CrowdDialogues::CrowdPartnerPoi(state.crowdHomePoiId))
            {
                if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(partnerPoiId))
                {
                    MoveTowardPoi(bot, state, poi, 8.0f);
                    state.crowdPhase = CROWD_WANDER_OUT;
                    state.crowdPhaseEndMs = now + urand(30, 120) * IN_MILLISECONDS;
                    return;
                }
            }
            state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
        }
        else
        {
            state.crowdPhaseEndMs = now + urand(20, 90) * IN_MILLISECONDS;
        }
    }

    void DriveDedicatedRole(Player* bot, CitizenState& state)
    {
        if (state.role == CITIZEN_ROLE_INN_DANCER)
        {
            DriveDancer(bot, state);
            return;
        }

        if (IsDuelerRole(state.role))
        {
            DriveDueler(bot, state);
            return;
        }

        if (state.role == CITIZEN_ROLE_FISHER)
        {
            DriveFisher(bot, state);
            return;
        }

        if (state.role == CITIZEN_ROLE_CROWD)
        {
            DriveCrowd(bot, state);
            return;
        }

        DriveAmbience(bot, state);
    }

    bool IsDuelHubCitizen(CitizenState const& state);

    void ReconcileRosterWithoutCitizenState(Player* bot, PlayerbotAI* botAI)
    {
        if (!bot || !botAI)
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        // Info, not Debug: this line is the ONLY evidence of which reconcile
        // arm a server actually takes, and it was invisible at the live
        // LogLevel while "citizens stand still" shipped twice.
        if (s_missingStateWarned.insert(guidLow).second)
            CbLog::Info("stage-cast bot {} (guid {}) has no citizen state; driving directly from roster",
                        bot->GetName(), guidLow);

        CitizenState& state = s_directRosterStates[guidLow];
        if (s_rosterStatePrimed.insert(guidLow).second || state.homeLocationKey.empty())
        {
            ApplyHomeLocation(bot, state);
            CbCitizenRoles::ApplyRosterRole(bot, state);
            if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId))
            {
                state.lastDirectRoamWasHub = IsAlternatingHubPoi(poi);
                state.stormwindRouteSeen = UsesAlternatingHubRoute(state.homeLocationKey);
                state.stormwindLastIssuedWasTrade = state.lastDirectRoamWasHub;
                s_stormwindDirectLastIssuedWasTrade[guidLow] = state.lastDirectRoamWasHub;
            }
        }

        StripPlayerbotsMaintenance(botAI);
        SuppressPlayerbotsChannelReplies(botAI);

        bool const duelHub = IsDuelHubCitizen(state);
        bool const goldshireDuelist = state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;
        EnsureDuelStrategy(botAI, true, duelHub, goldshireDuelist);
        if (IsDuelerRole(state.role))
            ApplyDuelerCombatStrategies(bot, botAI);

        // Frozen-city diagnostic: CanMutateWorld is only false while a teleport
        // is pending, so a wedged teleport parks the bot in the skip branch on
        // every sweep — with zero log evidence. A fresh staging teleport is in
        // flight for one sweep at most; warn once per blocked EPISODE when the
        // block has held for 30s straight.
        static std::unordered_map<uint32, uint32> s_mutateBlockedSinceMs;
        static std::unordered_set<uint32> s_mutateBlockedWarned;

        if (CbCitizenStateAccess::CanMutateWorld(bot))
        {
            s_mutateBlockedSinceMs.erase(guidLow);
            s_mutateBlockedWarned.erase(guidLow);

            EnsureStageCastOutfit(bot);
            if (!state.loginStaged)
            {
                // Fresh login: re-stage through the fixed resolver (see the
                // twin block in Reconcile). THIS is the path this server's
                // citizens actually take -- their AI state pointer is null, so
                // the with-state staging never ran and login positions saved
                // under the mesh were resumed verbatim. s_directRosterStates
                // outlives relogs, so ResetLoginStaging() re-arms this flag
                // from the login hook.
                state.loginStaged = true;
                CbLog::Info("staging {} at login (from {:.1f},{:.1f},{:.1f})",
                    bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                    bot->GetPositionZ());
                TeleportToHome(bot, state);
            }
            EnsureDedicatedStagePosition(bot, state);
            DriveDedicatedRole(bot, state);

            // Ongoing rescue on the arm this server actually takes: the
            // with-state arm has had this since 2026-08-12, but citizens here
            // never ran it — 17 were living on the Orgrimmar water table the
            // first night movement was un-frozen. The water clause: a
            // non-fisher citizen STANDING in water is always wrong (live: 5
            // ambient bots bobbing in the Valley of Honor pond) — and from
            // water the off-plane 60y net often misses them.
            // Water is now the PRIMARY net and applies moving or not: with
            // water-touching routes refused at the source, any non-fisher
            // citizen in liquid got there by a sink (or an old saved position)
            // -- and a swimmer on the water table has no walkable way out, so
            // waiting for it to stop only delays the rescue. Fishers and
            // docks/Goldshire pond dwellers are the legitimate exceptions.
            bool const inWaterWrongly =
                bot->IsInWater() && state.role != CITIZEN_ROLE_FISHER &&
                !(state.targetPoi.IsValid() &&
                  CityPoiRegistry::Instance().GetPoi(state.targetPoiId) &&
                  CityPoiRegistry::Instance().GetPoi(state.targetPoiId)->poiType == CITY_POI_DOCK);
            if (IsOffStagePlane(bot, state) || inWaterWrongly)
            {
                CbLog::Info("re-staging {} (guid {}): {} at ({:.1f},{:.1f},{:.1f})",
                            bot->GetName(), guidLow,
                            inWaterWrongly ? "in water" : "standing off the stage plane",
                            bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                TeleportToHome(bot, state);
            }
        }
        else
        {
            uint32& since = s_mutateBlockedSinceMs[guidLow];
            if (!since)
                since = getMSTime();
            else if (GetMSTimeDiffToNow(since) > 30 * IN_MILLISECONDS &&
                     s_mutateBlockedWarned.insert(guidLow).second)
                CbLog::Warn("not driving {} (guid {}): teleport pending for {}ms "
                            "(CanMutateWorld false on every sweep)",
                            bot->GetName(), guidLow, GetMSTimeDiffToNow(since));
        }

        if (duelHub && CbCitizenStateAccess::CanMutateWorld(bot))
            CbDuelBotUtil::EnsureLevel(bot);
    }

    bool IsAtHome(Player* bot, CitizenState const& state)
    {
        if (IsCitizenAccount(bot->GetSession()->GetAccountId()) && !state.homeLocationKey.empty())
            return CityLocationRegistry::Instance().IsBotInHomeLocation(bot, state.homeLocationKey.c_str());

        return CityPoiRegistry::Instance().IsCitizenZone(bot->GetZoneId());
    }

    bool IsDuelHubCitizen(CitizenState const& state)
    {
        CityLocationDef const* loc = CityLocationRegistry::Instance().FindByKey(state.homeLocationKey);
        return loc && loc->duelHub;
    }

    void PrimeRosterState(Player* bot, CitizenState& state)
    {
        if (!bot || !CitizenRosterRegistry::Instance().IsRosterGuid(bot->GetGUID().GetCounter()))
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        if ((state.initialized || s_rosterStatePrimed.find(guidLow) != s_rosterStatePrimed.end()) &&
            !state.homeLocationKey.empty())
            return;

        ApplyHomeLocation(bot, state);
        CbCitizenRoles::ApplyRosterRole(bot, state);
        s_rosterStatePrimed.insert(guidLow);
        if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.targetPoiId))
        {
            state.lastDirectRoamWasHub = IsAlternatingHubPoi(poi);
            state.stormwindRouteSeen = UsesAlternatingHubRoute(state.homeLocationKey);
            state.stormwindLastIssuedWasTrade = state.lastDirectRoamWasHub;
            s_stormwindDirectLastIssuedWasTrade[guidLow] = state.lastDirectRoamWasHub;
        }
    }
}

namespace CbStrategyGate
{
    void ResetLoginStaging(uint32 guidLow)
    {
        auto it = s_directRosterStates.find(guidLow);
        if (it != s_directRosterStates.end())
            it->second.loginStaged = false;
    }

    bool IsEligibleCitizen(Player* bot)
    {
        if (!bot || !CbSettings::GetBool("Enable"))
            return false;

        if (!bot->GetSession() || !bot->GetSession()->IsBot())
            return false;

        if (bot->InBattleground() || bot->GetGroup())
            return false;

        if (BotActivity::IsReserved(bot->GetGUID().GetCounter()))
        {
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            if (!botAI || !botAI->HasStrategy(kNonCombat, BOT_STATE_NON_COMBAT))
                return false;
        }

        uint32 guidLow = bot->GetGUID().GetCounter();
        uint32 accountId = bot->GetSession()->GetAccountId();

        if (CitizenRosterRegistry::Instance().IsRosterGuid(guidLow))
            return true;

        if (IsCitizenAccount(accountId) &&
            (CbCitizenLoginMgr::UsesDedicatedPool() || CbSettings::GetBool("UseDedicatedAccounts")))
        {
            return false;
        }

        if (CbSettings::GetUInt("Percent") == 0)
            return false;

        if (IsCitizenAccount(accountId))
            return false;

        if (!IsRndBotAccount(accountId) && !sRandomPlayerbotMgr.IsRandomBot(bot))
            return false;

        return CitizenHash(accountId, bot->GetGUID().GetCounter()) < CbSettings::GetUInt("Percent");
    }

    void Reconcile(Player* bot)
    {
        if (!bot || !CityBotsRuntime::ContextsRegistered())
            return;

        uint32 const guidLow = bot->GetGUID().GetCounter();
        bool const isRoster = CitizenRosterRegistry::Instance().IsRosterGuid(guidLow);
        if (isRoster)
        {
            CbPlayerbotsIntegration::RemoveFromRandomPlayerList(guidLow);
            SanitizeStageCastCompanions(bot);
        }

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        if (isRoster)
            SinkForensics(bot);

        // THE FROZEN-CITY ROOT CAUSE (found live 2026-08-16): roster citizens
        // are deliberately removed from the random-player list, so
        // RandomPlayerbotMgr never runs their AI update — and a bot teleport
        // only completes when the AI acks it (PlayerbotAI::HandleTeleportAck).
        // The 2026-08-12 login staging teleports EVERY citizen at login, so
        // every citizen wedged in IsBeingTeleported() forever ('.tele name'
        // answered "is already being teleported" 40 minutes after login),
        // CanMutateWorld() stayed false on every sweep, and every driver
        // silently skipped: the whole city stood still. Complete the ack here;
        // the bot lands and drives on the next sweep. This also un-wedges the
        // recorded far-continent placement hang — same missing ack.
        if (isRoster && bot->IsBeingTeleported())
        {
            botAI->HandleTeleportAck();
            CbLog::Info("completed pending teleport for {} (guid {})",
                        bot->GetName(), guidLow);
            return;
        }

        CitizenState* statePtr = CbCitizenStateAccess::Try(botAI);
        if (!statePtr)
        {
            if (isRoster)
                ReconcileRosterWithoutCitizenState(bot, botAI);
            return;
        }

        CitizenState& state = *statePtr;
        bool const canMutateWorld = CbCitizenStateAccess::CanMutateWorld(bot);
        PrimeRosterState(bot, state);

        if (isRoster)
        {
            StripPlayerbotsMaintenance(botAI);
            SuppressPlayerbotsChannelReplies(botAI);
            if (canMutateWorld)
            {
                EnsureStageCastOutfit(bot);
                if (!state.loginStaged)
                {
                    // Fresh login: re-stage the citizen at its POI through the
                    // fixed resolver. Positions saved while a citizen was under
                    // the mesh (the pre-fix era, or any future sink) would
                    // otherwise resume there -- swimming on the under-city
                    // water table with no path back to the stage.
                    state.loginStaged = true;
                    CbLog::Info("staging {} at login (from {:.1f},{:.1f},{:.1f})",
                        bot->GetName(), bot->GetPositionX(), bot->GetPositionY(),
                        bot->GetPositionZ());
                    TeleportToHome(bot, state);
                }
            }
        }

        bool eligible = IsEligibleCitizen(bot);
        bool atHome = IsAtHome(bot, state);
        bool shouldBeCitizen = eligible && (atHome || CbSettings::GetBool("TeleportIfOutsideCity"));

        bool hasNon = botAI->HasStrategy(kNonCombat, BOT_STATE_NON_COMBAT);
        bool hasCmb = botAI->HasStrategy(kCombat, BOT_STATE_COMBAT);

        Action nonAction = Decide(shouldBeCitizen, hasNon);
        Action cmbAction = Decide(shouldBeCitizen, hasCmb);
        bool duelHub = IsDuelHubCitizen(state);
        bool goldshireDuelist = false;
        if (CitizenState const* cs = CbCitizenStateAccess::Try(botAI))
            goldshireDuelist = cs->role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;

        if (nonAction == Action::None && cmbAction == Action::None)
        {
            if (shouldBeCitizen)
            {
                BotActivity::Reserve(guidLow);
                StripPlayerbotsMaintenance(botAI);
                SuppressPlayerbotsChannelReplies(botAI);
                EnsureDuelStrategy(botAI, shouldBeCitizen, duelHub, goldshireDuelist);
                if (IsDuelerRole(state.role))
                    ApplyDuelerCombatStrategies(bot, botAI);

                if (canMutateWorld)
                {
                    EnsureStageCastOutfit(bot);
                    EnsureDedicatedStagePosition(bot, state);
                    DriveDedicatedRole(bot, state);
                }
            }

            if (shouldBeCitizen && duelHub && canMutateWorld)
                CbDuelBotUtil::EnsureLevel(bot);

            if (shouldBeCitizen && canMutateWorld &&
                (IsSunkenBelowStage(bot, state) ||
                 (CbSettings::GetBool("TeleportIfOutsideCity") && !atHome)))
                TeleportToHome(bot, state);
            return;
        }

        if (nonAction == Action::Strip || cmbAction == Action::Strip)
        {
            BotActivity::Release(bot->GetGUID().GetCounter());
            if (!IsCitizenAccount(bot->GetSession()->GetAccountId()))
            {
                for (char const* strat : kRestoreNonCombat)
                {
                    if (!botAI->HasStrategy(strat, BOT_STATE_NON_COMBAT))
                        botAI->ChangeStrategy(std::string("+") + strat, BOT_STATE_NON_COMBAT);
                }
            }
        }

        if (nonAction == Action::Install)
        {
            StripPlayerbotsMaintenance(botAI);
            SuppressPlayerbotsChannelReplies(botAI);
            botAI->ChangeStrategy("+citizen", BOT_STATE_NON_COMBAT);
            BotActivity::Reserve(bot->GetGUID().GetCounter());

            ApplyHomeLocation(bot, state);
            CbCitizenRoles::ApplyRosterRole(bot, state);
            duelHub = IsDuelHubCitizen(state);
            goldshireDuelist = state.role == CITIZEN_ROLE_GOLDSHIRE_DUELIST;
            if (IsDuelerRole(state.role))
                ApplyDuelerCombatStrategies(bot, botAI);

            if (duelHub && canMutateWorld)
                CbDuelBotUtil::EnsureLevel(bot);

            if (canMutateWorld)
            {
                EnsureStageCastOutfit(bot);
                EnsureDedicatedStagePosition(bot, state);
                DriveDedicatedRole(bot, state);
            }

            if (canMutateWorld &&
                (IsSunkenBelowStage(bot, state) ||
                 (CbSettings::GetBool("TeleportIfOutsideCity") && !atHome)))
                TeleportToHome(bot, state);
        }

        if (shouldBeCitizen && (hasNon || nonAction == Action::Install) &&
            (duelHub || goldshireDuelist))
        {
            EnsureDuelStrategy(botAI, shouldBeCitizen, duelHub, goldshireDuelist);
            if (IsDuelerRole(state.role))
                ApplyDuelerCombatStrategies(bot, botAI);
        }

        if (cmbAction == Action::Install && !duelHub && !goldshireDuelist)
            botAI->ChangeStrategy("+citizen combat", BOT_STATE_COMBAT);

        if (nonAction == Action::Strip)
        {
            botAI->ChangeStrategy("-citizen", BOT_STATE_NON_COMBAT);
            if (botAI->HasStrategy("duel", BOT_STATE_NON_COMBAT))
                botAI->ChangeStrategy("-duel", BOT_STATE_NON_COMBAT);
        }

        if (cmbAction == Action::Strip)
            botAI->ChangeStrategy("-citizen combat", BOT_STATE_COMBAT);

        if (shouldBeCitizen && duelHub && botAI->HasStrategy(kCombat, BOT_STATE_COMBAT))
            botAI->ChangeStrategy("-citizen combat", BOT_STATE_COMBAT);

        if (shouldBeCitizen && goldshireDuelist && botAI->HasStrategy(kCombat, BOT_STATE_COMBAT))
            botAI->ChangeStrategy("-citizen combat", BOT_STATE_COMBAT);
    }

    void ReconcileAllBots()
    {
        for (auto const& [guid, player] : sRandomPlayerbotMgr.GetAllBots())
        {
            if (player && player->IsInWorld())
                Reconcile(player);
        }
    }

    void ReconcileRosterBots()
    {
        for (CitizenRosterEntry const& entry : CitizenRosterRegistry::Instance().All())
        {
            Player* bot = FindConnectedRosterBot(entry.guid);
            if (bot)
                Reconcile(bot);
        }
    }
}
