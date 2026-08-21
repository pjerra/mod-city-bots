# Living-crowd adversarial review — findings (Task 4)

Reviewed: full diff `main...feat/living-crowd` against
`docs/specs/2026-08-11-living-crowd-design.md`, five mandatory lenses.
Reviewer: Task 4 agent, 2026-08-12. Fixes applied in the same commit as this
report.

## Lens 1: claim leaks (claim -> release path enumeration)

Claim is created only in `CrowdBeginChat` (writes both sides atomically, no
early return between the two side writes; deadline = now + 45 s on both).
Release paths after the fixes:

| # | Path | Release |
|---|------|---------|
| I1 | Initiator ticks, deadline passed (any phase, asleep or awake, EnableSay on or off) | top-of-`Execute` full `CrowdReleaseChat` (FIXED — was non-initiator-only partial clear) |
| I2 | Initiator in LINGER, partner line due, partner gone/far (>20 yd) | `CrowdReleaseChat` + return |
| I3 | Exchange completes (`chatLineIdx >= lineCount`) | `CrowdReleaseChat` |
| I4 | Initiator logs out mid-exchange | its `CitizenState` dies with the PlayerbotAI; partner released by P2 |
| P1 | Partner side, conversation ends normally | initiator's `CrowdReleaseChat` clears partner (guarded: only if partner still points back) |
| P2 | Partner side, initiator stalls/vanishes | top-of-`Execute` full release at deadline (FIXED — now also puts partner on chat cooldown per spec) |
| P3 | Partner logs out mid-exchange | state dies with the AI; initiator released by I2 or I1 |
| — | Either side stops ticking entirely (combat/flee, stuck) | cannot chat anyway; counterpart frees itself via I1/P2; first tick after recovery releases via I1 |

No path exits the claim without a deadline-bounded release. Longest possible
claim lifetime is 45 s.

### Findings fixed (Important)

1. **Initiator-side stale claim could outlive the deadline indefinitely**
   (claim-leak class). The only initiator release sites lived inside the
   `EnableSay` chatter block, which is unreachable while asleep, while out of
   LINGER, or with `EnableSay = 0`. Toggling EnableSay off mid-exchange froze
   the initiator's claim forever (it could never chat again and could never be
   claimed). Fix: the top-of-`Execute` self-release now applies to BOTH roles
   and does a full `CrowdReleaseChat` — every claim dies at its deadline on
   the owner's next tick, unconditionally.
2. **Mid-conversation phase transition** (spec: "a claim is released when
   either citizen ... leaves LINGER"). On linger-timer expiry a conversing
   citizen (either side) could roll into AFK/WANDER, stalling the exchange
   mid-line and stranding the partner until deadline. Fix: the phase roll is
   deferred while `chatPartner` is set — a citizen in a conversation holds
   LINGER. No livelock: finding 1 bounds every claim to 45 s.
3. **`CrowdReleaseChat` left conversation residue on the other side**: it
   cleared the partner's `chatPartner`/`chatDeadlineMs` but not
   `chatInitiator`/`chatDialogueId`/`chatLineIdx`, so a partner releasing
   first left the initiator with `chatInitiator=true` + a live dialogue id and
   an empty partner guid (inert but wrong, and confusing to every later
   predicate). Fix: the guarded partner-side clear resets all five fields.

## Lens 2: AFK leaks

`ToggleAFK` set-site: exactly one (the AFK-entry roll, guarded by
`!bot->isAFK()`). Clear sites now cover every exit:

- state exit (`CROWD_AFK` timer expiry) — `CrowdClearAfk`;
- scene-asleep branch (also covers `EnableCrowdScenes = 0` rollback and
  burst-end, since the crowd action keeps ticking and clears on its first
  asleep tick) — `CrowdClearAfk`;
- `CitizenLogoutAction::Execute` — `CrowdClearAfk` before `LogoutPlayer`;
- `AbortCitizenLoginAttempt` (`CbCitizenLoginMgr.cpp`) — inline clear before
  `LogoutPlayerBot`;
- **(FIXED, Important)** the zombie-session logout in
  `CbPlayerbotsIntegration.cpp::TryLoginStageCastBot` also calls
  `LogoutPlayerBot` and had no clear — a crowd bot logged out `<AFK>`
  persists the player flag to the DB and would re-log flagged. Same two-line
  clear added; AFK-clear coverage over `LogoutPlayerBot`/`LogoutPlayer` call
  sites in the module is now exhaustive (3 of 3).

## Lens 3: API reality (compile risk)

Every call in the new code either already appears in this module or is the
plan-authorized `Player::ToggleAFK`/`Player::isAFK`:
`CbCitizenStateAccess::Try(GET_PLAYERBOT_AI(p))` (Goldshire duel finder),
`sRandomPlayerbotMgr.GetPlayerBot(ObjectGuid)` (`CbCitizenLoginMgr`),
`ObjectAccessor::GetPlayers()`, `SetFacingToObject`, `Say(.., LANG_UNIVERSAL)`,
`MoveNear(map,x,y,z,d)`, `CityPoiRegistry::GetPoi/GetWorldPosition`,
`urand/roll_chance_i/getMSTime/GetMSTimeDiffToNow`, `HandleEmoteCommand`.
`ObjectGuid::IsEmpty/Clear` come from `ObjectGuid.h`, already included by
`CitizenInfo.h` for the `chatPartner` field. `EMOTE_ONESHOT_TALK/POINT/LAUGH`
are used elsewhere in the module; `EMOTE_ONESHOT_NO`/`EMOTE_ONESHOT_QUESTION`
are standard `SharedDefines.h` Emote members (the plan's
`EMOTE_ONESHOT_SHRUG` does not exist in that enum — the Task 2 substitution
to `EMOTE_ONESHOT_QUESTION` was correct). `std::vector` reaches
`CitizenActions.cpp` via `CityPoiRegistry.h`. `poi->poiType` (not the plan's
illustrative `poi->type`) matches the real `CityPoi` field. POI ids 26/2
(Stormwind bank/AH) and 136/102 (Orgrimmar bank/AH) exist in
`CityPoiRegistry.cpp` with `locationKey` matching the assignment homes, so
`ApplyRosterRole`'s `locationKey == homeKey` check passes.
**(FIXED, Important)** `urand(CrowdAfkMinSec, CrowdAfkMaxSec)` with a
user-set Min > Max (ranges allow Min up to 600, Max down to 10) is undefined
behavior; the same pattern is guarded in `ActivityDurationMs`. Same clamp
added.

## Lens 4: behavior isolation

Non-crowd roles are byte-identical except: (a) `CitizenLogoutAction` and the
two login-manager logout sites gained an AFK clear that is a no-op for any
bot that is not AFK (only crowd bots ever set the flag); (b) wiring-only
additions in shared registry/context/strategy files; (c) the conf GUID moves
the spec mandates (net per-city citizen count unchanged, no GUID
double-listed — verified per-GUID, count 1 in each file). Dedicated-role
plumbing confirms isolation: `IsDedicated(CITIZEN_ROLE_CROWD)` is true, so
`ActivityExpired` is always false and `PickNextActivity` is a no-op — a
crowd citizen can never drift into another activity (which would otherwise
strand claims/AFK).

## Lens 5: spec deltas

Matching: knob defaults (1/60/90/60/180) in registry AND both conf files;
crowd lists 18+18 (spec ~15-20); partner scan 15 yd; reply pacing 3-6 s;
exchange 2-3 lines; both-sides cooldown on release; LINGER dwell 20-90 s;
AFK entry ~15%; WANDER dwell 30-120 s; `EnableSay` respected on top of crowd
gates; chatter pool by initiator's POI type with generic fallback; rollback
`EnableCrowdScenes = 0` verified inert (asleep branch clears AFK and stands
the bot down).

Deltas, all Minor / accepted:

- Dialogue table is 48 exchanges (spec "~50", plan floor >= 48; 16 bank /
  17 AH / 15 plaza — balanced).
- `CrowdSceneAwake` runs every crowd tick, not only at transition rolls
  (spec wording). Required anyway for the asleep early-return + AFK clear;
  same per-tick `FindNearbyRealPlayer` pattern the inn-dance action already
  uses; bounded at ~36 crowd citizens.
- WANDER visits only the bank<->AH partner POI, not the spec's "weighted
  ... vendor POIs" — plan-level simplification (`CrowdPartnerPoi`), accepted
  at plan time.

## Minors (recorded, not fixed)

1. Initiator lines (idx 0/2) are not guarded by partner presence — a partner
   who vanishes between lines can receive the closing line into thin air.
   Cosmetic; partner-line guard + 45 s deadline bound the exchange.
2. `CrowdPartnerPoi`/the assignment rotation hard-code POI ids 2/26/102/136;
   a DB-overridden POI table with different ids silently disables wander
   (falls back to linger). Accepted at plan time.
3. Raw `now >= deadline` uint32 comparisons mis-order across the 49.7-day
   `getMSTime` wrap (module-wide pre-existing pattern; nightly restart makes
   it moot).
4. Idle emotes can fire while a conversation is in progress (initiator
   gestures between lines). Arguably good; left as-is.
5. `CITIZEN_CROWD_SOCIAL` is not in `CitizenMovePoiTrigger`'s switch — not
   needed (the crowd TriggerNode carries its own `citizen move to poi`, same
   as plaza social's node), noted for symmetry only.
6. A scene falling asleep mid-WANDER resets the phase to LINGER at the
   partner POI; the citizen idles there until the next awake WANDER cycle
   walks it back. Self-healing, cosmetic.

## Verification deltas vs the plan's literal greps

- `grep -c CITIZEN_ROLE_CROWD CitizenRosterRegistry.cpp` is 2, not 1 — the
  plan itself prescribes two assignment rows in that one file.
- `grep -c "citizen crowd" CityBotsActionContext.h` is 1, not 2 — the
  creator is named `citizen_crowd` (underscore); registration + creator both
  present and read correct.
