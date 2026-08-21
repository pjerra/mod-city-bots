# mod-city-bots

`mod-city-bots` is an AzerothCore + mod-playerbots module that adds a fixed
"stage cast" of playerbots to cities and social hubs.

> **Credits:** created by **deadtrickz** — the whole concept and original
> implementation are theirs. This repository is a community-maintained fork
> with stability and movement fixes discovered while running it on a live
> server (see `CHANGELOG.md`). deadtrickz preferred not to host a repo, so it
> lives here. The goal is to make major
settlements feel populated without stealing slots from the normal random
playerbot population.

This module is currently built around a deterministic 400-bot roster:

| Resource | Reserved range |
| --- | --- |
| Auth accounts | `12001` through `12400` |
| Auth usernames | `citybot12001` through `citybot12400` |
| Auth password | `citybot_stage` |
| Character GUIDs | `9000001` through `9000400` |
| Roster table | `acore_playerbots.citizen_roster` |

Normal random playerbots log in first. City bots log in afterward, on top of the
configured random bot population.

## Requirements

- AzerothCore WotLK.
- `mod-playerbots` installed and enabled.
- Enough worldserver player capacity for normal playerbots plus city bots. For
  example, if `AiPlayerbot.MinRandomBots` and `AiPlayerbot.MaxRandomBots` are
  `1200`, set `PlayerLimit` high enough for at least `1200 + 400` city bots.

This is a playerbots extension, not a standalone NPC module.

## Install

1. Copy or clone this folder to:

   ```bash
   ~/azerothcore-wotlk/modules/mod-city-bots
   ```

2. Copy the config:

   ```bash
   cp ~/azerothcore-wotlk/modules/mod-city-bots/conf/mod_city_bots.conf.dist ~/azerothcore-wotlk/etc/mod_city_bots.conf
   ```

3. Rebuild `worldserver`.

4. Apply database updates.

   Preferred: let the AzerothCore database updater apply module SQL on
   worldserver startup. Do not also import those same files manually if you use
   the auto-updater.

   Manual install is also supported. Apply every file in each update directory
   in filename order, to the matching database:

   ```bash
   mysql -u acore -p acore_world < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-world/updates/2026_07_13_01_city_bot_poi.sql
   mysql -u acore -p acore_world < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-world/updates/2026_07_13_02_city_bot_ambiance.sql
   mysql -u acore -p acore_world < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-world/updates/2026_07_15_05_playercreateinfo_human_undead_hunter.sql

   mysql -u acore -p acore_auth < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-auth/updates/2026_07_16_03_stage_cast_one_account_per_bot.sql

   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_15_02_scale_stage_cast_400.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_15_04_fix_stage_cast_positions.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_16_05_expand_city_stage_targets.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_16_06_stage_cast_one_account_per_bot.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_16_07_cleanup_stage_cast_outfits_and_gate_positions.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_16_08_sync_stage_cast_race_classes.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_16_13_fix_stage_cast_login_rows.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_17_01_fix_stage_cast_required_login_rows.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_17_02_backfill_stage_cast_skills_reputation.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_17_03_fix_stage_cast_invalid_names.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_17_04_seed_stage_cast_basic_outfits.sql
   mysql -u acore -p acore_characters < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/db-characters/updates/2026_07_17_05_cleanup_stage_cast_randomizer_side_effects.sql

   mysql -u acore -p acore_playerbots < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/playerbots/updates/2026_07_15_00_citizen_roster.sql
   ```

   Development-only SQL patches may exist under `data/sql/dev` in the working
   repository. They are only for existing test databases while iterating. Do not
   include or apply `data/sql/dev` for a clean shared install; required final
   data is folded into the normal SQL files listed above.

5. Start `authserver`, then start `worldserver`.

6. Check the logs for lines like:

   ```text
   mod-city-bots: stage cast loaded: 400 roster entries
   mod-city-bots: playerbots autologin complete (... non-city bots), starting stage cast login
   mod-city-bots: status: ... | roster 400/400
   ```

## Default Cast

The shipped config keeps all 400 stage-cast city bots online after normal
playerbot autologin finishes. The SQL creates the 400 characters; the config
assignment sheet decides which GUID works each location/job. City-bot movement
and flavor actions are jittered per GUID so large groups do not all leave for
their next POI on the same server tick.

| Hub | Default target | Notes |
| --- | ---: | --- |
| Stormwind | 76 | Includes 2 static fishers |
| Goldshire | 35 | Includes 2 inn dancers, 2 duelers, 4 talkers, 2 fishers |
| Stormwind Gate | 22 | Permanent duel hub with paired duel reservations |
| Ironforge | 25 | Includes 2 static fishers |
| Darnassus | 20 | Alliance capital ambience |
| Exodar | 10 | Alliance capital ambience |
| Orgrimmar | 75 | Includes 2 static fishers |
| Orgrimmar Gate | 22 | Permanent duel hub with paired duel reservations |
| Undercity | 25 | Horde capital ambience |
| Thunder Bluff | 20 | Horde capital ambience |
| Silvermoon | 20 | Horde capital ambience |
| Shattrath | 20 | Fixed neutral hub |
| Dalaran | 30 | Fixed neutral hub |

- Living-crowd scenes: burst-gated bank/AH crowds in Stormwind and Orgrimmar with AFK cycles and paired /say chatter (see docs/specs/2026-08-11-living-crowd-design.md).

`CitizenBots.HybridPopulation = 1` (the default) keeps a low baseline
population (the `CityCount.Min.*` values) and bursts a hub up to its full
count while a real player is nearby. Set it to `0` to keep every hub at full
count at all times.

## Assignment Sheet

Every role is configurable by GUID in `mod_city_bots.conf`.

Static lists are permanent cast members:

```ini
CitizenBots.Assign.StormwindGate.Duelers = 9000001,9000002,...
CitizenBots.Assign.Goldshire.InnDancers = 9000025,9000026
CitizenBots.Assign.Stormwind.Fishers = 9000050,9000051
CitizenBots.Assign.Stormwind.Ambient = 9000052,9000053,...
```

Burst lists are optional extras for the same location/job:

```ini
CitizenBots.Assign.Stormwind.FishersBurst =
CitizenBots.Assign.Stormwind.AmbientBurst = 9000060,9000061,...
```

Duelers are intentionally static-only. Do not add dueler burst lists; gate duel
hubs are meant to stay populated at all times.

Fishers are also assignment-driven. Any bot running a fishing role learns
Fishing if needed and equips a fishing pole in the main-hand slot at runtime, so
the configured fisher GUIDs do not need hand-seeded poles in `character_inventory`.

General city ambience bots roam through the `city_bot_poi` table, not only their
starting `citizen_roster.poi_id`. Stormwind ships with 26 non-duel POIs covering
Trade District, Old Town, Dwarven District, Mage Quarter, canals, Cathedral
Square, the harbor, and the gates. Orgrimmar ships with 36 non-duel POIs spread
across the Valley of Strength, the Drag, Cleft of Shadow, Valley of Honor,
Valley of Spirits, Valley of Wisdom, ridges, and gates. Other capitals ship with
16 POIs each. Capital ambience routing alternates a commerce/social hub visit
with an outlying district visit. Stormwind uses Trade District as its hub,
Orgrimmar uses Valley of Strength, and the other capitals use their main
bank/auction/inn district groups.

`CitizenBots.Assign.RequireListed = 1` makes the config a strict casting sheet:
unlisted roster GUIDs do not log in. Set it to `0` if you want unlisted rows to
fall back to their `citizen_roster` database assignment.

## Important Config Notes

- `CitizenBots.Percent = 0` is intentional for this fixed-roster build. It means
  "do not convert normal random playerbots into city bots."
- `CitizenBots.UseDedicatedAccounts = 1` means the module uses the 400 dedicated
  citybot accounts seeded by SQL.
- `CitizenBots.DedicatedCount = 0` is a legacy fallback and is ignored when
  `CityCount.*` values are configured.
- `CitizenBots.LogLevel = 2` is the default: normal info/status logging. The
  Info lines are the module's only live evidence when something goes wrong —
  every root cause in `CHANGELOG.md` was found from them. Use `1` for
  warnings/errors only or `3` for debug (movement decision traces).
- `CitizenBots.LogMirrorToModule = 0` avoids duplicate console/log lines on
  servers that print both the `playerbots` and `module` logger categories.
- `CitizenBots.EnableSay = 1` with `CitizenBots.RandomSayChance = 0` is the
  shipped default: it enables ONLY the scripted living-crowd conversations
  (paired, cooldown-gated), not free chat. Keep `RandomSayChance = 0` — large
  bot crowds can create runaway chat cascades if city bots respond freely in
  `/say`. Set `EnableSay = 0` for fully silent citizens.
- `CitizenBots.EnableLogoutCycle = 0` keeps city bots permanent. They should not
  wander off or disappear as part of normal random-bot cycling.
- `CitizenBots.MountBetweenPoiChance = 90` lets non-dueler city bots mount while
  traveling between outdoor POIs. They dismount when they arrive.
- `CitizenBots.Assign.*` overrides `citizen_roster.home_key`, role, POI
  rotation, login priority, and burst/static status at server startup. This lets
  a server owner move a bot by editing config instead of editing SQL.
- Gate duelers are paired by the module before they duel. Free duelers reserve a
  compatible opponent, both move toward a shared meet point, then duel. This is
  intended to avoid the old "everyone runs around looking for someone" behavior.

## SQL Layout

The SQL is intentionally split by AzerothCore database:

| Directory | Database | Purpose |
| --- | --- | --- |
| `data/sql/db-auth/updates` | `acore_auth` | Dedicated citybot accounts |
| `data/sql/db-characters/updates` | `acore_characters` | Fixed characters, login rows, basic outfits |
| `data/sql/db-world/updates` | `acore_world` | POIs, ambience data, player creation compatibility |
| `data/sql/playerbots/updates` | `acore_playerbots` | `citizen_roster`, account type, playerbots integration flags |

For manual installs, apply files in filename order inside each directory.

## Updating an Existing Install

If you use the AzerothCore database updater, restart worldserver and let it apply
new module updates.

If you update manually, apply only the new SQL files that your server has not
already applied. Do not reapply old files unless the file was intentionally
changed and you know how your `updates` table tracks it.

## Troubleshooting

| Symptom or log | Likely fix |
| --- | --- |
| `city_bot_poi` does not exist | Apply all `db-world` SQL updates |
| `citizen_roster table empty` | Apply all `playerbots` SQL updates |
| `N stage cast characters missing` | Apply all `db-characters` SQL updates |
| Citybot accounts cannot log in | Apply all `db-auth` SQL updates |
| City bots start before random bots finish | Confirm current config has dedicated stage-cast mode and rebuild with this module |
| Stage cast stalls under very heavy random-bot load | Increase `CharacterDatabase.WorkerThreads` in `worldserver.conf` |
| Server freezes from citybot chatter | Keep `CitizenBots.EnableSay = 0` and `CitizenBots.RandomSayChance = 0` |
| Gate duelers stand around | Confirm gate POI SQL and playerbots roster SQL are applied, then check for stale duel flags or dead duelers in logs |
| Fishers stand together | Confirm current `city_bot_poi` has separate Goldshire dock POIs 416 and 417 |
| Fishers have no pole | Rebuild with current module code; fishers now equip item `6256` at runtime |

## Development

`tools/generate_stage_cast.py` is a developer helper for regenerating fixed
roster SQL. The server does not run Python files from this module.

Regenerating the roster can overwrite SQL assumptions, so do it deliberately and
review all generated database updates before sharing them.

## TODO

- Dalaran: tune POIs so the city feels busy, make all Dalaran bots level 80, and
  add sewer duelers.
- Fishing POIs: add one or two fixed fishers in each city with fishable water.
- City pathing: keep improving city-bot route quality inside cities.
- Mage duelers: investigate why they mostly cast only Fireball.

## License

[AGPL-3.0](LICENSE), matching AzerothCore and mod-playerbots.

## Uninstall

Uninstall scripts are in `data/sql/uninstall`. They are manual only and are not
run by the AzerothCore updater.

Read `data/sql/uninstall/README.md` before using them.
