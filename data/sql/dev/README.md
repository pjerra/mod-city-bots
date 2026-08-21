# Dev SQL patches

These files are for in-progress testing on an existing development server.
They are intentionally kept outside the AzerothCore install/update folders.

For a clean module install, use the SQL under:

| Directory | Database |
| --- | --- |
| `../db-auth/updates` | `acore_auth` |
| `../db-characters/updates` | `acore_characters` |
| `../db-world/updates` | `acore_world` |
| `../playerbots/updates` | `acore_playerbots` |

Current dev patch folders follow the same database split:

| Directory | Database |
| --- | --- |
| `db-characters/updates` | `acore_characters` |
| `db-world/updates` | `acore_world` |
| `playerbots/updates` | `acore_playerbots` |

When a dev patch becomes part of the module, fold the final state into the
normal install SQL too. Do not make users apply this folder for a fresh install.
