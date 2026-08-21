# Living-crowd scenes for capital bank/AH squares — design

Date: 2026-08-11. Status: approved by user (approach A), pending implementation.
Scope decisions were made interactively; the Q&A outcomes are folded in below.

## Goal

When a real player walks up to the bank or auction house in Stormwind or
Orgrimmar, the square should feel like a populated capital: a denser crowd of
citizens, some standing around, some flagged `<AFK>` like a player who tabbed
out, some drifting off and coming back, and pairs of citizens talking to each
other in `/say`. When no real player is near, none of this work happens.

## Decisions already made (user Q&A)

- **Crowd bodies come from the existing 400-citizen roster.** No new
  characters, accounts, or roster rows. Crowd citizens are re-assigned from
  the cities' existing `AmbientBurst` conf lists.
- **Chatter is canned dialogue pairs in code.** Themed pools, same pattern as
  the module's existing phrase arrays. No config-file lines, no LLM.
- **AFK is the full cycle**: linger → AFK flag → snap back → wander → return.
- **Stormwind + Orgrimmar only** in this iteration.
- **Architecture is approach A**: a new role/activity + one new action class
  inside the existing role→activity→action pipeline. No central scene
  director. No changes to other citizens' behavior.

## Non-goals (explicitly out of scope)

- Other capitals, Dalaran, Shattrath, Goldshire (Goldshire keeps its own
  plaza-social behavior untouched).
- Growing the roster; per-city crowd counts beyond what burst lists hold.
- Config-driven or generated dialogue content.
- Bot-to-player conversation (the existing greet-by-name behavior stays as-is
  and is not part of the crowd action).
- Any SQL/schema change. This feature is conf + C++ only.

## Architecture

### New role and assignment

- `CITIZEN_ROLE_CROWD` in `CbCitizenRoles` (dedicated role), mapped to a new
  activity `CITIZEN_CROWD_SOCIAL` by `ActivityForRole`.
- New conf keys, same shape and burst semantics as the existing lists:
  - `CitizenBots.Assign.Stormwind.CrowdBurst = <guid list>`
  - `CitizenBots.Assign.Orgrimmar.CrowdBurst = <guid list>`
- Population: move ~15–20 GUIDs per city out of `Assign.<City>.AmbientBurst`
  into `CrowdBurst`. Net citizen count per city is unchanged; a slice of the
  burst pool changes job. Duelers, fishers, dancers, talkers are untouched.
- Because crowd citizens are burst-only, hybrid population (enabled
  2026-08-11) already guarantees they are logged in only while a real player
  is in the hub zone, and log out `BurstGraceSec` after the last one leaves.

### New action: `CitizenCrowdAction`

One new file pair under `src/Ai/City/CityBots/Action/`. Per-citizen state
machine driven from the existing `CitizenState` (new fields, in-memory only):

- **LINGER** (base state): stand within short radius of the citizen's
  assigned bank/AH POI. Occasional one-shot emote from the existing emote
  table. Eligible to start or join chatter. Dwell 20–90 s, then roll a
  transition.
- **AFK**: set the real AFK player flag (the `<AFK>` tag players see), stop
  emoting and chatting, stand still for `CrowdAfkMinSec`–`CrowdAfkMaxSec`
  (defaults 60–180 s), then clear the flag and return to LINGER. Entry
  chance ~15% per transition roll. The flag MUST be cleared on: state exit,
  citizen logout, and burst deactivation (no permanently-AFK citizens).
- **WANDER**: pick another POI in the same hub (weighted toward bank, AH,
  and vendor POIs from the existing registry), walk there, dwell 30–120 s,
  walk back, return to LINGER. Uses the same movement calls the existing
  citizen actions use — no new pathing.

POI targeting: crowd citizens' POI selection is weighted to the hub's
`CITY_POI_BANK` and `CITY_POI_AUCTION_HOUSE` entries (both already exist in
`CityPoiRegistry` for both cities), so the crowd visibly clusters there.

### Bot-to-bot chatter (initiator-claim protocol)

- Dialogue content: a static table of ~50 short exchanges, 2–3 lines each,
  in themed pools: bank-queue talk, AH bargain talk, generic city small
  talk. Pool picked by the initiator's current POI type. Lines never
  interpolate player names (that stays the greeting action's job).
- Protocol: a LINGER citizen off chat-cooldown scans for a partner —
  another crowd citizen in LINGER, within 15 yd, not claimed. It claims the
  partner by writing both citizens' `CitizenState` conversation fields
  (partner guid + conversation deadline), the same shared-state claim
  pattern the duel code uses. The initiator then drives the whole exchange:
  says line 1, schedules the partner's reply 3–6 s later, optional third
  line, then releases the claim and puts BOTH citizens on
  `CrowdChatCooldownSec` (default 90 s).
- Claim safety: claims carry a deadline (exchange length + slack). A claim
  is released when the deadline passes, when either citizen logs out or
  leaves LINGER, or when burst ends. A citizen with a stale claim
  self-releases on its next tick. No claim survives a conversation that
  cannot finish.
- Chatter respects the existing global `CitizenBots.EnableSay` gate in
  addition to the new crowd gates below.

### The "near" gate

Zone-level burst decides who is logged in. On top of that, the expensive
liveliness (chatter, AFK transitions, wander transitions) runs ONLY when a
real player is within `CrowdPlayerRadius` (default 60 yd) of the citizen —
checked with the module's existing `FindNearbyRealPlayer` helper, at
transition rolls only (not every tick). No player near the square: the crowd
stands at its POIs doing nothing. Someone walks up: the square wakes.

### Config (all new keys, with defaults)

```
CitizenBots.EnableCrowdScenes = 1      # master off-switch
CitizenBots.CrowdPlayerRadius = 60
CitizenBots.CrowdChatCooldownSec = 90
CitizenBots.CrowdAfkMinSec = 60
CitizenBots.CrowdAfkMaxSec = 180
CitizenBots.Assign.Stormwind.CrowdBurst = <guids>
CitizenBots.Assign.Orgrimmar.CrowdBurst = <guids>
```

The two `CrowdBurst` lists are populated at implementation time by moving
15–20 GUIDs from the tail of each city's existing `AmbientBurst` list (the
tail choice is arbitrary and fine — ambient citizens are interchangeable);
the same GUIDs are removed from `AmbientBurst` so no citizen is double-listed.

Rollback at any time = `EnableCrowdScenes = 0` + restart; the citizens fall
back to plain ambient behavior (their role still resolves, the action goes
inert).

## Error handling

- Partner logs out / leaves mid-exchange → initiator's claim deadline
  expires, both sides self-release; no half-open conversations.
- Burst deactivates mid-scene → existing burst logout machinery runs;
  AFK flags are cleared in the logout path.
- A crowd GUID missing from the roster or not logged in → the assignment is
  skipped exactly as existing lists handle absent GUIDs.

## Performance and safety constraints

- No SQL, no new tables, no Lua (the hard `MapUpdate.Threads = 1` constraint
  with mod-ale is untouched).
- All new state is in-memory `CitizenState` fields; bounded by roster size.
- Actions run inside the existing playerbots AI tick and remain subject to
  mod-playerbots' BotActiveAlone throttling.
- The nearby-player check runs only at state-transition rolls, and only for
  ~30–40 crowd citizens total.

## Testing and rollout

Module C++ has no unit-test harness; the gates are:

1. **Compile gate**: the module builds inside the VM's existing worldserver
   docker build (same path the nightly rebuild uses).
2. **Live checklist** (user or supervised session, in-game at Stormwind
   bank/AH with a real character):
   - Crowd of ~15+ citizens present at bank/AH while standing there.
   - At least one paired `/say` exchange observed with natural pauses,
     and no citizen talking to itself or to two partners at once.
   - `<AFK>` tags appear on some citizens and clear again.
   - Citizens wander off and return.
   - Leave the zone: crowd logs out after the 180 s grace.
   - Return: crowd logs back in (burst re-activates).
   - With `EnableCrowdScenes = 0`: citizens behave like plain ambient.
3. **Deployment**: commit to `pjerra/mod-city-bots` → copy to the VM module
   dir → worldserver rebuild + restart (nightly window `dml-rebuild-59`
   ~23:59, or a user-approved manual restart). No restart without asking.

## Open questions

None — all scope questions were resolved in the Q&A above.
