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

For normal installs, prefer the AzerothCore database updater and do not import
the same update files manually afterward.

For manual installs, apply files in filename order within each directory. The
top-level `README.md` includes exact `mysql` commands for the current shipped
set.

The `uninstall` folder is manual only and is not run by the database updater.
