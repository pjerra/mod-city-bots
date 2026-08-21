# SQL layout

This module ships AzerothCore-style update files split by database:

| Directory | Database |
| --- | --- |
| `db-auth/updates` | `acore_auth` |
| `db-characters/updates` | `acore_characters` |
| `db-world/updates` | `acore_world` |
| `playerbots/updates` | `acore_playerbots` |

Development-only patches live under `dev/`. They are not part of the module
install path and should not be scanned by the AzerothCore database updater.
Use them only for existing test databases while iterating. When a dev patch
becomes required for clean installs, fold the final data back into the normal
`db-*` or `playerbots` update file too.

Keep normal install SQL consolidated. If a later update only repairs data from
an earlier module-owned seed, fold that final state back into the seed file and
move any existing-database repair SQL under `dev/` instead of shipping a fixup
chain.

For normal installs, let the AzerothCore database updater apply `db-auth`,
`db-characters` and `db-world`, and do not import those files manually
afterward. `playerbots/updates` is never auto-applied: mod-playerbots updates
`acore_playerbots` with its own loader, which does not scan other modules.
Import that file by hand before the first worldserver start.

The core updater runs auth, then characters, then world, and the playerbots
database last. `db-characters` files must therefore be self-contained: no
reads from `acore_playerbots` or `acore_world`, or a fresh install fails with
"table doesn't exist" before those databases are populated.

For manual installs, apply files in filename order within each directory, and
databases in the order playerbots, world, auth, characters. The top-level
`README.md` includes exact `mysql` commands for the current shipped set.

The `uninstall` folder is manual only and is not run by the database updater.
