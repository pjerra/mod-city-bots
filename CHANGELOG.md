# Changelog

The module was created by **deadtrickz**. This fork picks up from their build
as deployed in July 2026 and records everything changed since, with the reason
each change was needed. All fixes were found and verified on a live server
(~1,500 random playerbots + the 400-bot city cast).

## 2026-08-22 — fresh installs through the database updater

- **Fresh installs failed in the core updater** (issue #1, reported by
  DeadTrickz and notacoder-dev). Root cause, confirmed from the AzerothCore
  Playerbot-branch source: worldserver updates auth, then characters, then
  world, and mod-playerbots updates `acore_playerbots` last with its own
  loader that never scans other modules. Six `db-characters` updates joined
  `acore_playerbots.citizen_roster` (two also `acore_world.city_bot_poi`), so
  on an empty database the updater hit "table doesn't exist" and worldserver
  aborted. The C++ was never the crash; it already tolerates an empty roster.
- `db-characters` is now two self-contained seeds
  (`2026_08_22_00_stage_cast_characters.sql`, `2026_08_22_01_stage_cast_outfits.sql`)
  holding the final state the old 15-file chain produced. Verified on a throwaway
  MySQL 8.4 with the AC base schema: updater order on a fresh database succeeds
  and the resulting `acore_characters` is byte-identical to the old chain applied
  in the working order; reapplying is a no-op; an already-occupied equipment
  slot is left alone.
- `data/sql/playerbots/updates/2026_07_15_00_citizen_roster.sql` still has to be
  imported by hand before the first start; README now says so plainly instead
  of "preferred: auto-updater".
- `2026_08_11_01_sync_citizen_roster_race_classes.sql` moved under `data/sql/dev`
  (existing-database repair; its roster values were already in the roster seed).
- Startup log lines now name the shipped files instead of deleted ones
  (`src/CbShippedSql.h`); `conf/*.conf*` no longer point at a file that never
  shipped; `tools/generate_stage_cast.py` now writes to `tools/out/` only.
- Upgrading an existing server: the two new files run once and DELETE+INSERT
  the 400 stage-cast `characters` rows, homebinds and starter outfits, so the
  cast's saved positions, explored zones, taxi nodes and honor/kill counters
  reset to the shipped state. A customized roster needs the dev re-sync
  afterwards (README, Install step 4).
- Not yet done for this change: a worldserver boot with mod-playerbots against
  a freshly seeded database (verification was at the SQL/dump level).

## 2026-08 — the movement-hardening campaign

### The frozen-city fix (the big one)
- **Citizens froze completely** — staged at their spots, then never moved
  again. Root cause: roster citizens are removed from the random-bot list, so
  the playerbots engine never runs their AI update — and a bot teleport only
  completes when the AI acknowledges it. The login-staging teleport therefore
  wedged every citizen in "being teleported" forever, and every behavior
  driver silently skipped them. The gate now completes pending teleport acks
  itself. This also fixed the long-standing far-continent (Shattrath)
  placement hang — same missing ack.

### Water: the real "swimming under the city" mechanism
- Bots kept ending up on the water table under Orgrimmar and in the Stormwind
  canals. A day of movement forensics showed why: AzerothCore's pathfinder
  treats water polygons as walkable for players, so a citizen's shortest
  route across a city legitimately runs **through** ponds and canals — and
  from the under-city water plane there is no walkable way back out. Every
  move order the module issues now refuses a route whose points (or segment
  midpoints, or an incomplete path's tail) touch liquid.

### No more flying / wall-clipping
- When pathfinding failed, the engine's fallback walked bots in a straight
  line — through the air across Orgrimmar, through walls, through floors.
  Every module-issued move now requires a real navmesh path; long routes are
  walked in validated legs (following the mesh's own partial route, so ramps
  and bridges are used); unreachable spots are simply not traveled to.

### Self-healing rescues
- Any citizen standing below every nearby curated POI, standing in water
  (non-fishers), or unable to path anywhere three tries in a row is
  re-staged to its home spot within seconds — through a resolver that rejects
  under-floor and in-water placements, and prefers nearby on-level dry
  ground over exact coordinates (no more "standing inside the wall" after a
  rescue).

### Faction safety
- The config assignment sheet could place a citizen at an enemy-faction home
  (four Alliance citizens ended up as Orgrimmar-gate duelers → open faction
  PvP at the duel hub; Goldshire had Horde "residents" brawling with
  guards). Wrong-faction assignments are now refused with a log warning.
  Shattrath and Dalaran stay mixed, as neutral cities should.

### Living-crowd scenes (new feature)
- Burst-gated bank/auction-house crowds in Stormwind and Orgrimmar: citizens
  linger, go AFK, wander to the paired bank/AH and back, and hold short
  paired `/say` conversations — only while a real player is nearby, so idle
  servers pay nothing. Ported to the direct driving path so it works on
  playerbots forks where the per-bot AI context does not attach.

### Under-mesh staging fixes
- Teleport placement rejects height resolves that land under the floor (WMO
  cities have terrain and a water plane *underneath* the walkable floors);
  every citizen is re-staged through the fixed resolver at login.

### Dueler quality
- Level-80 boosted duelers used to fight with fresh-character spellbooks
  (4 spells at 80); they now get full class spellbooks on preparation.
- Real players walking near duel POIs are no longer movement-steered like
  citizens.

### Diagnostics that made all of this findable
- Per-bot status and staging lines at LogLevel 2; movement decision traces
  and plunge forensics (which order a sinking bot was following) at
  LogLevel 3. When something breaks, the log now says where and why.

## 2026-07 — deadtrickz's original build

The fixed 400-bot stage cast, deterministic SQL generation, per-city POI
tables, the assignment sheet, duel hubs with paired reservations, fishers,
inn dancers, plaza talkers — the whole design this fork builds on.
