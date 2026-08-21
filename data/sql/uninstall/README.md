# mod-city-bots uninstall

These scripts are manual only. They are not run by the AzerothCore database
updater.

## Steps

1. Stop `worldserver`.
2. Remove or disable the module from your build.
3. Run the world script against `acore_world`:

   ```bash
   mysql -u acore -p acore_world < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/uninstall/db-world/uninstall_mod_city_bots.sql
   ```

4. Run the playerbots script against `acore_playerbots`:

   ```bash
   mysql -u acore -p acore_playerbots < ~/azerothcore-wotlk/modules/mod-city-bots/data/sql/uninstall/playerbots/uninstall_mod_city_bots.sql
   ```

5. Start `worldserver`.

## What is removed

| Database | Removed |
| --- | --- |
| `acore_world` | `city_bot_poi` table, module-owned guard SmartAI rows, and human/undead hunter playercreateinfo compatibility rows |
| `acore_playerbots` | `citizen_roster`, citizen stage-cast queue/state rows, and account type `3` markers for accounts `12001`-`12400` |

## What is not removed

- Citybot accounts in `acore_auth`.
- Citybot characters in `acore_characters`.
- `mod_city_bots.conf` in your server `etc` directory.

Remove those manually only if you are sure no later reinstall should reuse the
fixed account and GUID ranges.
