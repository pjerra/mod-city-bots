# Changelog

The module was created by **deadtrickz**. This fork picks up from their build
as deployed in July 2026 and records everything changed since, with the reason
each change was needed. All fixes were found and verified on a live server
(~1,500 random playerbots + the 400-bot city cast).

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
