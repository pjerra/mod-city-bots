# Living-Crowd Scenes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bank/AH squares in Stormwind and Orgrimmar gain a burst-gated crowd of citizens that AFK-cycle, wander, and hold paired `/say` conversations — only while a real player is near.

**Architecture:** Approach A from `docs/specs/2026-08-11-living-crowd-design.md`: one new citizen role (`CITIZEN_ROLE_CROWD`) + activity (`CITIZEN_CROWD_SOCIAL`) wired through the existing role→activity→trigger→action pipeline, one new action class (`CitizenCrowdAction`) carrying a LINGER/AFK/WANDER state machine and an initiator-claim chatter protocol. Conf-only crowd assignment from existing burst lists. No SQL, no Lua, no new threads.

**Tech Stack:** AzerothCore module C++ (WotLK 3.3.5, mod-playerbots strategy/action framework), worldserver `.conf` settings.

## Global Constraints

- **No compiler on the authoring machine.** The module only compiles inside the VM's worldserver docker build. Every task's verification is static (grep/consistency checks); the compile gate is the final VM rebuild. Write conservative C++: only APIs already used in this codebase, plus `Player::ToggleAFK()` / `Player::isAFK()`.
- **No SQL, no schema changes, no Lua.** (`MapUpdate.Threads = 1` constraint must stay untouched.)
- **Do not modify behavior of existing roles/actions.** Goldshire plaza-social, duelers, fishers, dancers stay byte-identical except where a task explicitly lists an edit.
- **All new conf keys use the `CitizenBots.` prefix** and must be registered in `kCbSettings` (unregistered keys are not readable via `CbSettings`).
- Spec: `docs/specs/2026-08-11-living-crowd-design.md`. Defaults: `EnableCrowdScenes=1`, `CrowdPlayerRadius=60`, `CrowdChatCooldownSec=90`, `CrowdAfkMinSec=60`, `CrowdAfkMaxSec=180`.
- Commit after every task with a `feat(crowd):` / `docs(crowd):` prefix. Work on branch `feat/living-crowd`.

---

### Task 1: Plumbing — role, activity, settings, wiring, conf

**Files:**
- Modify: `src/Ai/City/CityBots/CitizenInfo.h` (role + activity enums, CitizenState fields)
- Modify: `src/CbCitizenRoles.cpp` (ActivityForRole)
- Modify: `src/Ai/City/CityBots/Settings/CbSettingsRegistry.h` (new keys)
- Modify: `src/CitizenRosterRegistry.cpp` (two AssignmentDef rows)
- Modify: `src/Ai/City/CityBots/Strategy/CitizenStrategy.cpp` (trigger node)
- Modify: `src/Ai/City/CityBots/CityBotsTriggerContext.h` (trigger creator)
- Modify: `src/Ai/City/CityBots/CityBotsActionContext.h` (action creator)
- Modify: `src/Ai/City/CityBots/Action/CitizenActions.h` (stub action class decl)
- Modify: `src/Ai/City/CityBots/Action/CitizenActions.cpp` (stub action impl)
- Modify: `conf/mod_city_bots.conf.dist` and `conf/mod_city_bots.conf` (keys + GUID list moves)

**Interfaces:**
- Consumes: existing enums/registries as listed.
- Produces: `CITIZEN_ROLE_CROWD` (=6), `CITIZEN_CROWD_SOCIAL` (=18), settings keys `EnableCrowdScenes`, `CrowdPlayerRadius`, `CrowdChatCooldownSec`, `CrowdAfkMinSec`, `CrowdAfkMaxSec`, trigger name `"citizen crowd status"`, action name `"citizen crowd"`, class `CitizenCrowdAction` (stub — Task 2 fills it), CitizenState crowd/chat fields (exact list below — Tasks 2 and 3 rely on these names).

- [ ] **Step 1: Enums + state fields** — in `CitizenInfo.h` add to `CitizenRole`:

```cpp
    CITIZEN_ROLE_FISHER = 5,
    CITIZEN_ROLE_CROWD = 6
```

add to `CitizenActivity`:

```cpp
    CITIZEN_PLAZA_SOCIAL = 17,
    CITIZEN_CROWD_SOCIAL = 18
```

add to `struct CitizenState` (after `mountAttemptMs`):

```cpp
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
```

- [ ] **Step 2: Role mapping** — in `CbCitizenRoles.cpp`, `ActivityForRole` switch, add before `default:`:

```cpp
            case CITIZEN_ROLE_CROWD:
                return CITIZEN_CROWD_SOCIAL;
```

- [ ] **Step 3: Settings** — in `CbSettingsRegistry.h` append to `kCbSettings` (before the closing `};`):

```cpp
    { "EnableCrowdScenes",       CbType::Bool,  1,   0,   1   },
    { "CrowdPlayerRadius",       CbType::UInt,  60,  10,  200 },
    { "CrowdChatCooldownSec",    CbType::UInt,  90,  10, 3600 },
    { "CrowdAfkMinSec",          CbType::UInt,  60,  10,  600 },
    { "CrowdAfkMaxSec",          CbType::UInt,  180, 10, 1200 },
```

- [ ] **Step 4: Assignment rows** — in `CitizenRosterRegistry.cpp`, inside the `assignments` vector, directly after the `"Assign.Stormwind.AmbientBurst"` row add:

```cpp
            { "Assign.Stormwind.CrowdBurst", "Stormwind", CITIZEN_ROLE_CROWD, true,
              { 26, 2 } },
```

and directly after the `"Assign.Orgrimmar.AmbientBurst"` row add:

```cpp
            { "Assign.Orgrimmar.CrowdBurst", "Orgrimmar", CITIZEN_ROLE_CROWD, true,
              { 136, 102 } },
```

(POI rotation ids: Stormwind bank=26 / AH=2, Orgrimmar bank=136 / AH=102 — already present in `CityPoiRegistry.cpp`. Round-robin assignment puts half the crowd at each.)

- [ ] **Step 5: Trigger + action wiring** — `CitizenStrategy.cpp`, after the `"citizen plaza social status"` TriggerNode add:

```cpp
    triggers.push_back(new TriggerNode("citizen crowd status",
        { NextAction("citizen move to poi", 3.0f), NextAction("citizen crowd", 6.0f) }));
```

`CityBotsTriggerContext.h`: register `creators["citizen crowd status"] = &CityBotsTriggerContext::crowd;` next to `plaza_social`, and add the private creator:

```cpp
    static Trigger* crowd(PlayerbotAI* ai)
    {
        return new CitizenActivityTrigger(ai, "citizen crowd status", CITIZEN_CROWD_SOCIAL);
    }
```

`CityBotsActionContext.h`: register `creators["citizen crowd"] = &CityBotsActionContext::citizen_crowd;` and add:

```cpp
    static Action* citizen_crowd(PlayerbotAI* ai) { return new CitizenCrowdAction(ai); }
```

- [ ] **Step 6: Stub action** — in `CitizenActions.h`, next to `CitizenPlazaSocialAction`, declare (same shape as its neighbors):

```cpp
class CitizenCrowdAction : public CitizenBaseAction
{
public:
    CitizenCrowdAction(PlayerbotAI* botAI) : CitizenBaseAction(botAI, "citizen crowd") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};
```

(Match the exact base-class constructor signature used by `CitizenPlazaSocialAction` in this header — copy its pattern verbatim, only the name and action string differ.) In `CitizenActions.cpp` add a minimal implementation at the end of the file:

```cpp
bool CitizenCrowdAction::isUseful()
{
    CitizenState const& state = State();
    return state.activity == CITIZEN_CROWD_SOCIAL && state.atPoi &&
           !bot->HasMountedAura() && !bot->isMoving();
}

bool CitizenCrowdAction::Execute(Event /*event*/)
{
    return true; // Task 2 replaces this with the crowd state machine
}
```

- [ ] **Step 7: Conf lists** — in BOTH `conf/mod_city_bots.conf` and `conf/mod_city_bots.conf.dist`:
  - Remove GUIDs `9000108`–`9000125` (18) from `CitizenBots.Assign.Stormwind.AmbientBurst`, and from `CitizenBots.Assign.Stormwind.Ambient` if that list contains any of them.
  - Remove GUIDs `9000183`–`9000200` (18) from `CitizenBots.Assign.Orgrimmar.AmbientBurst`, and from `CitizenBots.Assign.Orgrimmar.Ambient` likewise.
  - Add, next to the burst lists they came from:

```
CitizenBots.Assign.Stormwind.CrowdBurst = 9000108,9000109,9000110,9000111,9000112,9000113,9000114,9000115,9000116,9000117,9000118,9000119,9000120,9000121,9000122,9000123,9000124,9000125
CitizenBots.Assign.Orgrimmar.CrowdBurst = 9000183,9000184,9000185,9000186,9000187,9000188,9000189,9000190,9000191,9000192,9000193,9000194,9000195,9000196,9000197,9000198,9000199,9000200
```

  - Add the new keys near the other CitizenBots toggles, with a comment block:

```
# Living-crowd scenes (bank/AH squares; see docs/specs/2026-08-11-living-crowd-design.md)
CitizenBots.EnableCrowdScenes = 1
CitizenBots.CrowdPlayerRadius = 60
CitizenBots.CrowdChatCooldownSec = 90
CitizenBots.CrowdAfkMinSec = 60
CitizenBots.CrowdAfkMaxSec = 180
```

- [ ] **Step 8: Verify (static)** — all must hold:
  - `grep -c "CITIZEN_ROLE_CROWD" src/Ai/City/CityBots/CitizenInfo.h src/CbCitizenRoles.cpp src/CitizenRosterRegistry.cpp` → 1 each.
  - `grep -c "citizen crowd" src/Ai/City/CityBots/CityBotsActionContext.h` → 2 (registration + creator); same check on `CityBotsTriggerContext.h` for `"citizen crowd status"` → 2; `CitizenStrategy.cpp` → 1.
  - No GUID appears in two conf lists: for each moved GUID, `grep -c <guid> conf/mod_city_bots.conf` → 1.
  - `9000121` and `9000196` are in the CrowdBurst lists (they are repaired ex-mages — fine, they are ordinary citizens now).

- [ ] **Step 9: Commit** — `git add -A && git commit -m "feat(crowd): role, activity, settings, wiring, conf lists (stub action)"`

---

### Task 2: `CitizenCrowdAction` state machine (LINGER / AFK / WANDER)

**Files:**
- Modify: `src/Ai/City/CityBots/Action/CitizenActions.cpp` (replace the Task 1 stub `Execute`; add file-local helpers)

**Interfaces:**
- Consumes: CitizenState crowd fields from Task 1 (`crowdPhase`, `crowdPhaseEndMs`, `crowdHomePoiId`); settings keys from Task 1; existing helpers in this file: `FindNearbyRealPlayer(Player*, float)`, `CityBotSayAllowed(CitizenState&)`, `MoveNear(...)` (via base class), `CityPoiRegistry::Instance().GetPoi(id)` / `.GetWorldPosition(poi)`.
- Produces: phase constants and transition behavior Task 3 hooks chatter into: chatter runs only in phase 0 (LINGER). A file-local helper `CrowdSceneAwake(Player* bot)` (real player within `CrowdPlayerRadius`) that Task 3 reuses.

- [ ] **Step 1: Helpers** — near the top of `CitizenActions.cpp`'s anonymous namespace add:

```cpp
    enum CrowdPhase : uint8
    {
        CROWD_LINGER = 0,
        CROWD_AFK = 1,
        CROWD_WANDER_OUT = 2,
        CROWD_WANDER_BACK = 3
    };

    bool CrowdSceneAwake(Player* bot)
    {
        if (!CbSettings::GetBool("EnableCrowdScenes"))
            return false;
        return FindNearbyRealPlayer(bot,
            static_cast<float>(CbSettings::GetUInt("CrowdPlayerRadius"))) != nullptr;
    }

    // bank <-> AH partner POI per city (ids from CityPoiRegistry)
    uint32 CrowdPartnerPoi(uint32 poiId)
    {
        switch (poiId)
        {
            case 26:  return 2;    // Stormwind bank -> AH
            case 2:   return 26;   // Stormwind AH -> bank
            case 136: return 102;  // Orgrimmar bank -> AH
            case 102: return 136;  // Orgrimmar AH -> bank
            default:  return 0;
        }
    }

    void CrowdClearAfk(Player* bot)
    {
        if (bot->isAFK())
            bot->ToggleAFK();
    }
```

(`FindNearbyRealPlayer` already exists file-locally at the top of this file — do not duplicate it; if it sits below the new code, move the whole existing helper up unchanged.)

- [ ] **Step 2: Replace the stub `Execute`** with the state machine:

```cpp
bool CitizenCrowdAction::Execute(Event /*event*/)
{
    CitizenState& state = State();
    uint32 const now = getMSTime();

    if (state.sitting)
    {
        bot->SetStandState(UNIT_STAND_STATE_STAND);
        state.sitting = false;
    }

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

    // LINGER: chatter hook goes here (Task 3).

    if (now < state.crowdPhaseEndMs)
    {
        // Occasional idle emote while lingering.
        if (roll_chance_i(10) && GetMSTimeDiffToNow(state.lastEmoteMs) > 25 * IN_MILLISECONDS)
        {
            state.lastEmoteMs = now;
            static uint32 const idleEmotes[] = {
                EMOTE_ONESHOT_TALK, EMOTE_ONESHOT_NO, EMOTE_ONESHOT_POINT,
                EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_SHRUG
            };
            bot->HandleEmoteCommand(idleEmotes[urand(0, 4)]);
        }
        return true;
    }

    // Linger expired: roll the next phase. 15% AFK, 20% wander, else linger on.
    uint32 const roll = urand(1, 100);
    if (roll <= 15)
    {
        if (!bot->isAFK())
            bot->ToggleAFK();
        state.crowdPhase = CROWD_AFK;
        state.crowdPhaseEndMs = now + urand(CbSettings::GetUInt("CrowdAfkMinSec"),
                                            CbSettings::GetUInt("CrowdAfkMaxSec")) * IN_MILLISECONDS;
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
```

- [ ] **Step 3: AFK safety on logout/burst-end** — find the citizen logout/cleanup path (`CitizenLogoutAction::Execute` in this file and/or the burst logout in `CbCitizenLoginMgr.cpp`); at the point the citizen is about to log out add `CrowdClearAfk(bot);` (or inline `if (bot->isAFK()) bot->ToggleAFK();` where the helper is out of scope). Both paths must clear the flag.

- [ ] **Step 4: Verify (static)** — `grep -n "ToggleAFK" src/` shows: set-site in the state machine, clear in `CrowdClearAfk`, clear at each logout path (≥2 clear sites). `grep -c "CROWD_LINGER" src/Ai/City/CityBots/Action/CitizenActions.cpp` ≥ 4. No other action file changed: `git diff --stat` touches only `CitizenActions.cpp` (+ `CbCitizenLoginMgr.cpp` if the logout clear landed there).

- [ ] **Step 5: Commit** — `git commit -am "feat(crowd): LINGER/AFK/WANDER state machine in CitizenCrowdAction"`

---

### Task 3: Paired chatter (dialogue table + initiator-claim protocol)

**Files:**
- Modify: `src/Ai/City/CityBots/Action/CitizenActions.cpp` (dialogue table + chatter logic in the LINGER branch)

**Interfaces:**
- Consumes: CitizenState chat fields from Task 1 (`chatPartner`, `chatDeadlineMs`, `chatLineIdx`, `chatDialogueId`, `chatInitiator`, `lastCrowdChatMs`); `CrowdSceneAwake`; `CbSettings` keys; the module's per-citizen state accessor used by the other actions in this file (the same mechanism `State()` uses — chatter must read the PARTNER's `CitizenState` through the module's state registry, `CbCitizenStateAccess`).
- Produces: complete feature; nothing downstream.

- [ ] **Step 1: Dialogue table** — file-local, above `CitizenCrowdAction::Execute`:

```cpp
    struct CrowdDialogue
    {
        CityPoiType pool;           // CITY_POI_BANK, CITY_POI_AUCTION_HOUSE, or CITY_POI_PLAZA (= generic)
        char const* lines[3];       // alternating: initiator, partner, initiator
        uint8 lineCount;            // 2 or 3
    };

    static CrowdDialogue const kCrowdDialogues[] =
    {
        { CITY_POI_BANK, { "This queue gets longer every day.", "Tell me about it. I just need one deposit box.", nullptr }, 2 },
        { CITY_POI_BANK, { "Guild vault's full again.", "Stop hoarding shoulder armor then.", "It's a collection!" }, 3 },
        { CITY_POI_BANK, { "They should open a second counter.", "They've been saying that since the war ended.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Saronite prices are madness this week.", "Buy now. They only go up before raid night.", nullptr }, 2 },
        { CITY_POI_AUCTION_HOUSE, { "Someone undercut me by one copper. One!", "That's just how the game is played, friend.", "Then I'll undercut by two." }, 3 },
        { CITY_POI_AUCTION_HOUSE, { "Any good deals today?", "Glyphs are cheap. Everything else, forget it.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "Weather's turning.", "Aye. Good day to stay in the city.", nullptr }, 2 },
        { CITY_POI_PLAZA, { "You hear about the trouble up north?", "Everyone has. Glad it's someone else's problem.", "For now." }, 3 },
        // Task agent: extend to >= 48 exchanges total, same three pools, same tone —
        // short WoW-flavored small talk, no player names, no lore-breaking references,
        // roughly balanced across the three pools.
    };
```

- [ ] **Step 2: Chatter logic** — replace the `// LINGER: chatter hook goes here (Task 3).` comment with:

```cpp
    // --- paired chatter (initiator drives both sides) ---
    if (CbSettings::GetBool("EnableSay"))
    {
        // Ongoing conversation I initiated: deliver the next line when due.
        if (state.chatInitiator && state.chatDialogueId >= 0)
        {
            if (now >= state.chatDeadlineMs)
                CrowdReleaseChat(state);            // partner vanished or exchange overran
            else if (now >= state.crowdPhaseEndMs - 1)
            { /* phase timing untouched; line pacing below uses its own stamps */ }

            CrowdDialogue const& d = kCrowdDialogues[state.chatDialogueId];
            if (state.chatLineIdx < d.lineCount && now >= state.lastSayMs + urand(3000, 6000))
            {
                bool const partnerLine = (state.chatLineIdx % 2) == 1;
                Player* speaker = bot;
                if (partnerLine)
                {
                    speaker = ObjectAccessor::FindPlayer(state.chatPartner);
                    if (!speaker || !speaker->IsInWorld() ||
                        bot->GetDistance(speaker) > 20.0f)
                    {
                        CrowdReleaseChat(state);
                        return true;
                    }
                }
                speaker->Say(d.lines[state.chatLineIdx], LANG_UNIVERSAL);
                state.lastSayMs = now;
                ++state.chatLineIdx;
                if (state.chatLineIdx >= d.lineCount)
                    CrowdReleaseChat(state);
            }
        }
        // Idle and off cooldown: try to start one.
        else if (!state.chatPartner &&
                 GetMSTimeDiffToNow(state.lastCrowdChatMs) >
                     CbSettings::GetUInt("CrowdChatCooldownSec") * IN_MILLISECONDS &&
                 roll_chance_i(25))
        {
            if (Player* partner = CrowdFindChatPartner(bot, state))
                CrowdBeginChat(bot, partner, state, now);
        }
    }
```

- [ ] **Step 3: Protocol helpers** — file-local, near the dialogue table (uses the same partner-state access pattern as the existing code in `CbCitizenStateAccess.h` — read that header first and use its actual accessor names):

```cpp
    void CrowdReleaseChat(CitizenState& state)
    {
        if (CitizenState* partnerState = CbCitizenStateAccess::Find(state.chatPartner))
        {
            partnerState->chatPartner.Clear();
            partnerState->chatDeadlineMs = 0;
            partnerState->lastCrowdChatMs = getMSTime();
        }
        state.chatPartner.Clear();
        state.chatInitiator = false;
        state.chatDialogueId = -1;
        state.chatLineIdx = 0;
        state.chatDeadlineMs = 0;
        state.lastCrowdChatMs = getMSTime();
    }

    Player* CrowdFindChatPartner(Player* bot, CitizenState const& state)
    {
        for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
        {
            if (!player || player == bot || !player->GetSession() ||
                !player->GetSession()->IsBot())
                continue;
            CitizenState* ps = CbCitizenStateAccess::Find(player->GetGUID());
            if (!ps || ps->role != CITIZEN_ROLE_CROWD || ps->crowdPhase != CROWD_LINGER ||
                ps->chatPartner || ps->activity != CITIZEN_CROWD_SOCIAL)
                continue;
            if (bot->GetDistance(player) > 15.0f)
                continue;
            return player;
        }
        return nullptr;
    }

    void CrowdBeginChat(Player* bot, Player* partner, CitizenState& state, uint32 now)
    {
        CitizenState* ps = CbCitizenStateAccess::Find(partner->GetGUID());
        if (!ps)
            return;

        // Pool by the initiator's home POI type; PLAZA pool is the generic fallback.
        CityPoiType pool = CITY_POI_PLAZA;
        if (CityPoi const* poi = CityPoiRegistry::Instance().GetPoi(state.crowdHomePoiId))
            if (poi->type == CITY_POI_BANK || poi->type == CITY_POI_AUCTION_HOUSE)
                pool = static_cast<CityPoiType>(poi->type);

        std::vector<int16> candidates;
        for (int16 i = 0; i < static_cast<int16>(std::size(kCrowdDialogues)); ++i)
            if (kCrowdDialogues[i].pool == pool || kCrowdDialogues[i].pool == CITY_POI_PLAZA)
                candidates.push_back(i);
        if (candidates.empty())
            return;

        state.chatDialogueId = candidates[urand(0, candidates.size() - 1)];
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
```

**IMPORTANT:** `CbCitizenStateAccess::Find` is illustrative — Task agent MUST open `src/CbCitizenStateAccess.h`/`.cpp`, use the real accessor (name, key type, return type), and adjust these helpers accordingly. If no by-guid accessor exists, add one to `CbCitizenStateAccess` following its existing style. A partner claimed but never released is the bug class to defend against: every early return between claim and release must go through `CrowdReleaseChat`.

- [ ] **Step 4: Stale-claim self-release** — at the top of `Execute` (right after the `sitting` reset) add:

```cpp
    if (state.chatPartner && !state.chatInitiator && now >= state.chatDeadlineMs)
    {
        state.chatPartner.Clear();
        state.chatDeadlineMs = 0;
    }
```

- [ ] **Step 5: Fill the dialogue table** to ≥ 48 exchanges (balanced pools, tone per the seed lines, `lineCount` correct for every entry, no `{}` interpolation anywhere).

- [ ] **Step 6: Verify (static)** —
  - Every `return` path between `CrowdBeginChat` and conversation end passes through `CrowdReleaseChat` (read the diff line-by-line; this is the review anchor).
  - `grep -c "CrowdDialogue" src/Ai/City/CityBots/Action/CitizenActions.cpp` ≥ 3; dialogue entry count ≥ 48 (`grep -c "CITY_POI_" ...` on the table block).
  - `grep -n "Say(" src/Ai/City/CityBots/Action/CitizenActions.cpp` — new Say sites use `d.lines[...]`, no `StringFormat`, no player names.

- [ ] **Step 7: Commit** — `git commit -am "feat(crowd): paired /say chatter with initiator-claim protocol + dialogue pools"`

---

### Task 4: Adversarial review vs spec + fix wave

**Files:** read-only pass over the full diff `git diff main...feat/living-crowd`, then fixes in place.

- [ ] **Step 1: Review** the complete diff against `docs/specs/2026-08-11-living-crowd-design.md` with these mandatory lenses:
  1. **Claim leaks** — enumerate every code path from claim to release; any path that can exit without release is a Critical.
  2. **AFK leaks** — every way a citizen can log out / change activity / lose burst while `isAFK()`; each must clear the flag.
  3. **API reality** — every AzerothCore/playerbots call used must already appear elsewhere in the module OR be `Player::ToggleAFK`/`Player::isAFK`; flag anything else as a compile risk.
  4. **Behavior isolation** — `git diff` must show zero behavior change for non-crowd roles (wiring-only edits allowed in shared files).
  5. **Spec deltas** — knob defaults, radii, cooldowns, list sizes vs spec numbers.
- [ ] **Step 2: Fix** every Critical/Important finding in place; note Minors in the review report. Re-run Task 2/3 static verifies after fixing.
- [ ] **Step 3: Commit** — `git commit -am "fix(crowd): review fixes (<n> findings)"` and write the findings list into `docs/plans/2026-08-11-living-crowd-review.md`.

---

### Task 5: Deployment artifacts

**Files:**
- Create: `docs/plans/2026-08-11-living-crowd-deploy.md`
- Modify: `README.md` (one feature bullet under the module's feature list)

- [ ] **Step 1: Deploy doc** — write `docs/plans/2026-08-11-living-crowd-deploy.md` containing exactly: (a) the scp commands to copy the changed files from this repo to `<your-server>` (list each changed file explicitly from `git diff --name-only main...feat/living-crowd`); (b) the note that the worldserver must REBUILD (C++ change — the nightly `dml-rebuild-59` window ~23:59 covers it) and that conf changes ALSO need copying to the container path `/azerothcore/env/dist/etc/modules/mod_city_bots.conf` (docker exec cp from the mounted module dir or sed the same edits); (c) the live checklist copied verbatim from the spec's "Testing and rollout" section; (d) rollback: `CitizenBots.EnableCrowdScenes = 0` + restart.
- [ ] **Step 2: README** — add one bullet: `- Living-crowd scenes: burst-gated bank/AH crowds in Stormwind and Orgrimmar with AFK cycles and paired /say chatter (see docs/specs/2026-08-11-living-crowd-design.md).`
- [ ] **Step 3: Commit** — `git commit -am "docs(crowd): deployment guide + README feature bullet"`

---

## Self-review notes (done at plan time)

- Spec coverage: assignment→T1, state machine→T2, chatter→T3, near-gate→T2 (`CrowdSceneAwake`), error handling→T2 S3 + T3 S3/S4, config→T1, testing/rollout→T5 + Global Constraints. Non-goals respected (no SQL/Lua/other cities).
- Known open risk, accepted: `CbCitizenStateAccess` accessor shape is verified by the Task 3 agent against the real header (flagged IMPORTANT in-task); the compile gate is the VM rebuild, not local.
- Type consistency: CitizenState field names in T1 S1 match every use in T2/T3.
