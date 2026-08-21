#!/usr/bin/env python3
"""
Historical generator of the mod-city-bots V2 stage-cast identity data.

Outputs go to tools/out/<db>/ (git-ignored) for reference only:
  - db-auth:       fixed account IDs 12001+
  - db-characters: fixed character GUIDs 9000001+ (characters rows only)
  - playerbots:    citizen_roster + account_type rows

It does NOT produce the shipped seeds. data/sql/db-characters/updates/* are
dumps of the final database state (characters, homebinds, outfits) and
data/sql/playerbots/updates/2026_07_15_00_citizen_roster.sql has been edited by
hand since this tool last ran. Never copy tools/out files into
data/sql/*/updates: the AzerothCore updater applies every new file there, and
db-characters files must stay self-contained (no reads from acore_playerbots /
acore_world). See data/sql/README.md.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

# ---------------------------------------------------------------------------
# Reserved ID ranges (document in README — do not overlap on your realm)
# ---------------------------------------------------------------------------
ACCOUNT_ID_START = 12001
CHARS_PER_ACCOUNT = 1
CAST_CAPACITY = 400
ACCOUNT_COUNT = CAST_CAPACITY // CHARS_PER_ACCOUNT
GUID_START = 9_000_001
ACCOUNT_PREFIX = "citybot"
ACCOUNT_PASSWORD = "citybot_stage"  # same on every install

MODULE_ROOT = Path(__file__).resolve().parents[1]
OUT_ROOT = MODULE_ROOT / "tools/out"  # never data/sql/*/updates (auto-applied by the core updater)
OUT_AUTH = OUT_ROOT / "db-auth"
OUT_CHARS = OUT_ROOT / "db-characters"
OUT_PB = OUT_ROOT / "playerbots"

# SRP6 constants (WoW / AzerothCore)
SRP_N = int(
    "894B645E89E1535BBDAD5BFCBDE65FB578DAD451A3FCEB876E126FAFDB4434", 16
)
SRP_G = 7

# race: 1 human 3 dwarf 4 gnome 7 night elf 11 draenei
#       2 orc 5 undead 6 tauren 8 troll 10 blood elf
# class: 1 warrior 2 paladin 3 hunter 4 rogue 5 priest 7 shaman 8 mage 9 warlock 11 druid

# role: 0 FLEX 1 INN_DANCER 2 PLAZA_TALKER 3 GATE_DUELIST 4 GOLDSHIRE_DUELIST

# Roster slots per home (must sum to CAST_CAPACITY). Exceeds CityCount max for headroom.
ROSTER_QUOTAS: dict[str, int] = {
    "StormwindGate": 22,
    "OrgrimmarGate": 22,
    "Goldshire": 35,
    "Stormwind": 76,
    "Orgrimmar": 75,
    "Dalaran": 30,
    "Shattrath": 20,
    "Ironforge": 25,
    "Undercity": 25,
    "Darnassus": 20,
    "Exodar": 10,
    "ThunderBluff": 20,
    "Silvermoon": 20,
}

assert sum(ROSTER_QUOTAS.values()) == CAST_CAPACITY

# Alliance / Horde / neutral race sets for flex citizens
ALLIANCE_RACES = (1, 3, 4, 7, 11)
HORDE_RACES = (2, 5, 6, 8, 10)
NEUTRAL_RACES = (1, 10, 11, 2, 5)

HOME_TEAM = {
    "Stormwind": ALLIANCE_RACES,
    "Ironforge": ALLIANCE_RACES,
    "Darnassus": ALLIANCE_RACES,
    "Exodar": ALLIANCE_RACES,
    "Goldshire": ALLIANCE_RACES,
    "StormwindGate": ALLIANCE_RACES,
    "Orgrimmar": HORDE_RACES,
    "Undercity": HORDE_RACES,
    "ThunderBluff": HORDE_RACES,
    "Silvermoon": HORDE_RACES,
    "OrgrimmarGate": HORDE_RACES,
    "Shattrath": NEUTRAL_RACES,
    "Dalaran": NEUTRAL_RACES,
}

HOME_PREFIX = {
    "Stormwind": "Sw",
    "Ironforge": "If",
    "Darnassus": "Da",
    "Exodar": "Ex",
    "Goldshire": "Gs",
    "Orgrimmar": "Or",
    "Undercity": "Uc",
    "ThunderBluff": "Tb",
    "Silvermoon": "Sm",
    "Shattrath": "Sh",
    "Dalaran": "Dl",
}

HOME_POI_BASE = {
    "Stormwind": 1,
    "Ironforge": 201,
    "Darnassus": 701,
    "Exodar": 801,
    "Goldshire": 401,
    "Orgrimmar": 101,
    "Undercity": 301,
    "ThunderBluff": 901,
    "Silvermoon": 1001,
    "Shattrath": 1101,
    "Dalaran": 1201,
}

RACE_NAME_PARTS = {
    1: (("Al", "Bran", "Ced", "Daw", "El", "Garr", "Hal", "Jor", "Mar", "Perr", "Row", "Tav", "Wes", "Ar"), ("ric", "den", "wyn", "ley", "ton", "ford", "ren", "wick", "lan", "mond")),
    2: (("Gar", "Gor", "Kaz", "Mok", "Naz", "Rok", "Skarn", "Thrak", "Urok", "Vark", "Zug", "Drog"), ("ash", "goth", "mak", "rok", "gar", "vek", "gul", "nak", "mog")),
    3: (("Brom", "Dun", "Far", "Grim", "Har", "Krag", "Thar", "Tor", "Ulf", "Yor", "Bar", "Dorn"), ("din", "gar", "grim", "rik", "dan", "mar", "stag", "bar", "ren")),
    4: (("Ari", "Bel", "Del", "Fey", "Leth", "Maer", "Nara", "Syl", "Thal", "Vey", "Elar", "Sari"), ("dorn", "lara", "riel", "thera", "wyn", "dris", "nara", "song", "leaf")),
    5: (("Alar", "Bly", "Cald", "Drel", "Ed", "Fen", "Mor", "Neth", "Os", "Vel", "Wes", "Hest"), ("ric", "win", "grave", "mere", "wick", "ton", "ford", "well", "row")),
    6: (("Apon", "Boru", "Hamu", "Kodo", "Mah", "Noru", "Ranu", "Tahu", "Yetu", "Bano"), ("horn", "mane", "run", "hoof", "totem", "cloud", "wind", "plain")),
    7: (("Bix", "Cog", "Fizz", "Gim", "Jix", "Nib", "Pip", "Tink", "Wix", "Zim", "Mek", "Fiz"), ("bit", "bolt", "fizz", "gear", "lock", "spark", "whiz", "wire", "snap")),
    8: (("Aka", "Bwem", "Jaz", "Jin", "Kaz", "Rok", "Sen", "Taz", "Vol", "Zan", "Zul", "Mek"), ("jin", "ra", "zul", "tik", "za", "mon", "rek", "ji", "dak")),
    10: (("Ael", "Bel", "Cael", "Felo", "Lora", "Sel", "Talar", "Vale", "Vel", "Zan", "Ren", "Sae"), ("ion", "aris", "ther", "rien", "lara", "drel", "anor", "riel", "vyn")),
    11: (("Aal", "Eli", "Ish", "Kyr", "Mara", "Naar", "Orel", "Sama", "Vey", "Yrel", "Daan", "Lumi"), ("ira", "aan", "dor", "iel", "uun", "ara", "esh", "ion", "ria")),
}

# poi_id -> (x, y, z, map_id, zone_id) — mirrors data/sql/db-world city_bot_poi
POI_SPAWN = {
    1: (-8867.0, 673.0, 97.9, 0, 1519),
    2: (-8813.0, 663.0, 95.0, 0, 1519),
    3: (-8815.0, 667.0, 95.0, 0, 1519),
    4: (-8865.0, 650.0, 96.0, 0, 1519),
    5: (-8835.0, 625.0, 94.0, 0, 1519),
    101: (1634.0, -4445.0, 15.0, 1, 1637),
    102: (1679.0, -4455.0, 18.0, 1, 1637),
    103: (1622.0, -4390.0, 10.0, 1, 1637),
    104: (1640.0, -4430.0, 15.0, 1, 1637),
    201: (-4845.0, -880.0, 502.0, 0, 1537),
    202: (-4960.0, -910.0, 503.0, 0, 1537),
    203: (-4850.0, -850.0, 502.0, 0, 1537),
    301: (1633.0, 240.0, -43.0, 0, 1497),
    302: (1600.0, 170.0, -56.0, 0, 1497),
    401: (-9469.0, 62.0, 56.0, 0, 12),
    402: (-9466.0, 58.0, 56.0, 0, 12),
    403: (-9458.0, 68.0, 56.0, 0, 12),
    404: (-9472.58, 37.34, 64.69, 0, 12),
    405: (-9474.0, 39.0, 64.69, 0, 12),
    406: (-9470.0, 35.0, 64.69, 0, 12),
    407: (-9462.0, 72.0, 56.0, 0, 12),
    408: (-9455.0, 68.0, 56.0, 0, 12),
    409: (-9468.0, 75.0, 56.0, 0, 12),
    501: (-9083.5, 377.5, 93.5, 0, 12),
    502: (-9075.0, 372.0, 93.5, 0, 12),
    503: (-9092.0, 372.0, 93.5, 0, 12),
    504: (-9075.0, 383.0, 93.5, 0, 12),
    505: (-9092.0, 383.0, 93.5, 0, 12),
    506: (-9083.5, 390.0, 93.5, 0, 12),
    601: (1404.0, -4400.0, 25.0, 1, 14),
    602: (1380.0, -4380.0, 26.0, 1, 14),
    603: (1420.0, -4385.0, 25.0, 1, 14),
    604: (1390.0, -4420.0, 25.0, 1, 14),
    605: (1365.0, -4395.0, 26.0, 1, 14),
    606: (1410.0, -4415.0, 25.0, 1, 14),
    701: (9947.0, 2483.0, 1316.0, 1, 1657),
    801: (-3965.0, -11653.0, -139.0, 530, 3557),
    901: (-1196.0, 28.0, 177.0, 1, 1638),
    1001: (9480.0, -7279.0, 14.0, 530, 3487),
    1101: (-1838.0, 5301.0, -12.0, 530, 3703),
    1201: (5807.0, 588.0, 661.0, 571, 4395),
}

# Race/class pairs valid for WotLK. DKs are avoided in generated ambience rows
# so the stage cast does not need DK starter-state/bootstrap data.
RACE_CLASSES: dict[int, tuple[int, ...]] = {
    1: (1, 2, 3, 4, 5, 6, 8, 9),
    2: (1, 3, 4, 6, 7, 9),
    3: (1, 2, 3, 4, 5, 6),
    4: (1, 3, 4, 5, 6, 11),
    5: (1, 3, 4, 5, 6, 8, 9),
    6: (1, 3, 6, 7, 11),
    7: (1, 4, 6, 8, 9),
    8: (1, 3, 4, 5, 6, 7, 8),
    10: (2, 3, 4, 5, 6, 8, 9),
    11: (1, 2, 3, 5, 6, 7, 8),
}


def pick_class(race: int, level: int, index: int) -> int:
    classes = list(RACE_CLASSES[race])
    if level < 55:
        classes = [c for c in classes if c != 6]
    return classes[index % len(classes)]


def make_stage_name(race: int, index: int, used_names: set[str]) -> str:
    starts, ends = RACE_NAME_PARTS[race]
    for spin in range(len(starts) * len(ends)):
        name = f"{starts[(index + spin) % len(starts)]}{ends[((index // len(starts)) + spin) % len(ends)]}"
        name = name[:12]
        key = name.lower()
        if key not in used_names and 2 <= len(name) <= 12:
            return name
    raise SystemExit(f"Could not generate unique name for race={race} index={index}")


def cast_entries():
    entries = []
    prio = 0
    used_names: set[str] = set()

    def add(name, race, cls, gender, level, home, role, poi, priority=None):
        nonlocal prio
        if cls not in RACE_CLASSES.get(race, ()):
            raise SystemExit(
                f"Invalid race/class for {name}: race={race} class={cls}"
            )
        if not (2 <= len(name) <= 12) or not name.isalpha():
            raise SystemExit(f"Invalid player name for {name}")
        if name.lower() in used_names:
            raise SystemExit(f"Duplicate stage-cast name {name}")
        p = priority if priority is not None else prio
        prio += 1
        used_names.add(name.lower())
        entries.append((name, race, cls, gender, level, home, role, poi, p))

    # Stormwind gate duelists (22) — always level 80
    gate_names = [
        "Aldgate", "Brenwick", "Corven", "Daxley", "Eldon", "Farris",
        "Garrick", "Hadrian", "Iven", "Jorick", "Kelden", "Lareth",
        "Marlen", "Nerris", "Osric", "Perrin", "Quillan", "Rowan",
        "Sable", "Tavian", "Ulric", "Veyra",
    ]
    gate_pois = [
        501, 503, 501, 504, 505, 502, 501, 503, 506, 504, 505, 502,
        501, 502, 503, 504, 505, 506, 501, 502, 503, 504,
    ]
    gate_race_classes = [
        (1, 1), (3, 2), (3, 3), (7, 4), (1, 5), (7, 8),
        (1, 9), (11, 2), (11, 7), (4, 11), (7, 4), (3, 1),
        (1, 2), (7, 8), (4, 1), (7, 6), (11, 7), (1, 6),
        (3, 4), (4, 11), (7, 8), (11, 2),
    ]
    for i, n in enumerate(gate_names):
        race, cls = gate_race_classes[i]
        add(n, race, cls, i % 2, 80, "StormwindGate", 3, gate_pois[i], i)

    # Orgrimmar gate duelists (22) — always level 80
    org_names = [
        "Thrakka", "Mokren", "Zulvek", "Grashna", "Rokmar", "Durgoth",
        "Hazrak", "Korgath", "Mazrek", "Nazgul", "Orgrim", "Pazzik",
        "Qargul", "Ruzak", "Skarn", "Tazgur", "Urokh", "Varkul",
        "Wrogg", "Xurga", "Yazgrim", "Zorvak",
    ]
    org_gate_pois = [
        601, 602, 603, 604, 605, 606, 601, 602, 603, 604, 605, 606,
        601, 602, 603, 604, 605, 606, 601, 602, 603, 604,
    ]
    org_gate_cast = [
        (2, 1), (2, 7), (2, 3), (2, 4), (2, 9), (8, 3),
        (8, 7), (8, 8), (5, 5), (5, 9), (10, 2), (10, 4),
        (2, 6), (8, 7), (5, 6), (10, 5), (2, 7), (8, 1),
        (5, 4), (10, 9), (2, 3), (8, 8),
    ]
    for i, n in enumerate(org_names):
        race, cls = org_gate_cast[i]
        add(n, race, cls, i % 2, 80, "OrgrimmarGate", 3, org_gate_pois[i], 20 + i)

    # Goldshire dedicated (8 of 25) — inn dancers on Lion's Pride upstairs beds (POI 404-405)
    add("Liora", 1, 5, 1, 20, "Goldshire", 1, 404, 40)
    add("Mira", 1, 8, 1, 22, "Goldshire", 1, 405, 41)
    add("Denton", 1, 1, 0, 25, "Goldshire", 4, 408, 42)
    add("Rylee", 1, 4, 0, 24, "Goldshire", 4, 409, 43)
    add("Tomas", 1, 1, 0, 18, "Goldshire", 2, 407, 44)
    add("Ella", 1, 5, 1, 19, "Goldshire", 2, 403, 45)
    add("Finn", 1, 8, 0, 21, "Goldshire", 2, 407, 46)
    add("Greta", 1, 3, 1, 20, "Goldshire", 2, 408, 47)

    dedicated_by_home: dict[str, int] = {
        "StormwindGate": 22,
        "OrgrimmarGate": 22,
        "Goldshire": 8,
    }

    for home, quota in ROSTER_QUOTAS.items():
        if home in ("StormwindGate", "OrgrimmarGate"):
            continue

        dedicated = dedicated_by_home.get(home, 0)
        flex_count = quota - dedicated
        if flex_count <= 0:
            continue

        races = HOME_TEAM[home]
        poi_base = HOME_POI_BASE[home]

        for i in range(flex_count):
            race = races[i % len(races)]
            name = make_stage_name(race, len(entries) + i, used_names)
            gender = i % 2
            if home == "Goldshire":
                level = 12 + (i % 20)
            elif home in ("Dalaran", "Shattrath"):
                level = 30 + (i % 40)
            else:
                level = 15 + (i % 50)
            cls = pick_class(race, level, i)

            add(
                name,
                race,
                cls,
                gender,
                level,
                home,
                0,
                poi_base + (i % 15),
            )

    if len(entries) != CAST_CAPACITY:
        raise SystemExit(
            f"Cast size {len(entries)} != CAST_CAPACITY {CAST_CAPACITY}"
        )

    return entries


def _bignum_mod_pow(base: int, exp: int, mod: int) -> int:
    return pow(base, exp, mod)


def srp6_verifier(username: str, password: str, account_id: int) -> tuple[bytes, bytes]:
    """Deterministic salt from account_id so every install matches."""
    salt = hashlib.sha256(
        f"mod-city-bots-v2-salt:{account_id}:{username}".encode()
    ).digest()

    inner = hashlib.sha256(f"{username.upper()}:{password.upper()}".encode()).digest()
    x = int.from_bytes(
        hashlib.sha256(salt + inner).digest(), "little"
    )
    v = _bignum_mod_pow(SRP_G, x, SRP_N)
    verifier = v.to_bytes(32, "little")
    return salt, verifier


def spawn_for_poi(poi_id: int, home: str, guid: int) -> tuple[float, float, float, int, int]:
    import math

    if poi_id in POI_SPAWN:
        x, y, z, map_id, zone = POI_SPAWN[poi_id]
    else:
        x, y, z, map_id, zone = spawn_for_home(home)

    radius = ((guid % 10) + 2) * 0.75
    angle = math.radians(guid % 360)
    return x + math.cos(angle) * radius, y + math.sin(angle) * radius, z, map_id, zone


def spawn_for_home(home: str) -> tuple[float, float, float, int, int]:
    spawns = {
        "StormwindGate": (-9083.5, 377.5, 93.5, 0, 12),
        "OrgrimmarGate": (1404.0, -4400.0, 25.0, 1, 14),
        "Goldshire": (-9465.0, 62.0, 56.0, 0, 12),
        "Stormwind": (-8833.0, 622.0, 94.0, 0, 1519),
        "Ironforge": (-4981.0, -881.0, 502.0, 0, 1537),
        "Darnassus": (9862.0, 2491.0, 1316.0, 1, 1657),
        "Exodar": (-3965.0, -11653.0, -139.0, 530, 3557),
        "Orgrimmar": (1676.0, -4315.0, 61.0, 1, 1637),
        "Undercity": (1633.0, 240.0, 62.0, 0, 1497),
        "ThunderBluff": (-1274.0, 116.0, 131.0, 1, 1638),
        "Silvermoon": (9739.0, -7365.0, 14.0, 530, 3487),
        "Shattrath": (-1838.0, 5301.0, -12.0, 530, 3703),
        "Dalaran": (5807.0, 588.0, 660.0, 571, 4395),
    }
    return spawns.get(home, (-8833.0, 622.0, 94.0, 0, 1519))


def generate():
    cast = cast_entries()
    if len(cast) > ACCOUNT_COUNT * CHARS_PER_ACCOUNT:
        raise SystemExit(
            f"Cast size {len(cast)} exceeds {ACCOUNT_COUNT * CHARS_PER_ACCOUNT} slots"
        )

    OUT_AUTH.mkdir(parents=True, exist_ok=True)
    OUT_CHARS.mkdir(parents=True, exist_ok=True)
    OUT_PB.mkdir(parents=True, exist_ok=True)

    auth_lines = [
        "-- mod-city-bots V2 stage cast — auth accounts (fixed IDs)",
        "-- Password for every account: citybot_stage",
        f"DELETE FROM `account` WHERE `id` >= {ACCOUNT_ID_START} "
        f"AND `id` < {ACCOUNT_ID_START + ACCOUNT_COUNT};",
        "",
    ]

    for i in range(ACCOUNT_COUNT):
        aid = ACCOUNT_ID_START + i
        user = f"{ACCOUNT_PREFIX}{aid}"
        salt, verifier = srp6_verifier(user, ACCOUNT_PASSWORD, aid)
        auth_lines.append(
            "INSERT INTO `account` (`id`, `username`, `salt`, `verifier`, `expansion`, "
            "`Locale`, `os`, `recruiter`, `locked`, `online`) VALUES "
            f"({aid}, '{user}', UNHEX('{salt.hex()}'), UNHEX('{verifier.hex()}'), "
            "2, 0, '', 0, 0, 0);"
        )

    auth_lines.append("")
    auth_lines.append("ALTER TABLE `account` AUTO_INCREMENT = 12100;")

    char_lines = [
        "-- mod-city-bots V2 stage cast — characters (fixed GUIDs)",
        f"DELETE FROM `characters` WHERE `guid` >= {GUID_START} "
        f"AND `guid` < {GUID_START + CAST_CAPACITY};",
        "",
    ]

    pb_roster = [
        "-- mod-city-bots V2 — citizen_roster (fixed cast)",
        "DROP TABLE IF EXISTS `citizen_roster`;",
        """CREATE TABLE `citizen_roster` (
    `guid` INT UNSIGNED NOT NULL,
    `account_id` INT UNSIGNED NOT NULL,
    `character_name` VARCHAR(12) NOT NULL,
    `race` TINYINT UNSIGNED NOT NULL,
    `class` TINYINT UNSIGNED NOT NULL,
    `gender` TINYINT UNSIGNED NOT NULL,
    `level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    `home_key` VARCHAR(32) NOT NULL,
    `role` TINYINT UNSIGNED NOT NULL DEFAULT 0,
    `poi_id` INT UNSIGNED NOT NULL DEFAULT 0,
    `login_priority` SMALLINT NOT NULL DEFAULT 100,
    `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
    PRIMARY KEY (`guid`),
    UNIQUE KEY `idx_name` (`character_name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;""",
        "",
        "DELETE FROM `citizen_roster`;",
        "",
    ]

    pb_accounts = [
        "-- Mark stage cast accounts as AddClass type (2) so playerbots can admit them on demand",
        f"DELETE FROM `playerbots_account_type` WHERE `account_id` >= {ACCOUNT_ID_START} "
        f"AND `account_id` < {ACCOUNT_ID_START + ACCOUNT_COUNT};",
        "",
    ]

    for i in range(ACCOUNT_COUNT):
        aid = ACCOUNT_ID_START + i
        pb_accounts.append(
            "INSERT INTO `playerbots_account_type` (`account_id`, `account_type`, `assignment_date`) "
            f"VALUES ({aid}, 2, NOW());"
        )

    pb_accounts.append("")

    guid = GUID_START
    for idx, row in enumerate(cast):
        name, race, cls, gender, level, home, role, poi, prio = row
        account_id = ACCOUNT_ID_START + (idx // CHARS_PER_ACCOUNT)
        x, y, z, map_id, zone = spawn_for_poi(poi, home, guid)

        skin = guid % 5
        face = guid % 10
        hair = guid % 15
        hcolor = guid % 8
        facial = guid % 7

        char_lines.append(
            "INSERT INTO `characters` (`guid`, `account`, `name`, `race`, `class`, `gender`, "
            "`level`, `xp`, `money`, `skin`, `face`, `hairStyle`, `hairColor`, `facialStyle`, "
            "`bankSlots`, `restState`, `playerFlags`, `map`, `instance_id`, `instance_mode_mask`, "
            "`position_x`, `position_y`, `position_z`, `orientation`, `trans_x`, `trans_y`, "
            "`trans_z`, `trans_o`, `transguid`, `taximask`, `cinematic`, `totaltime`, `leveltime`, "
            "`rest_bonus`, `logout_time`, `is_logout_resting`, `resettalents_cost`, "
            "`resettalents_time`, `extra_flags`, `stable_slots`, `at_login`, `zone`, "
            "`death_expire_time`, `taxi_path`, `arenaPoints`, `totalHonorPoints`, "
            "`todayHonorPoints`, `yesterdayHonorPoints`, `totalKills`, `todayKills`, "
            "`yesterdayKills`, `chosenTitle`, `knownCurrencies`, `watchedFaction`, `drunk`, "
            "`health`, `power1`, `power2`, `power3`, `power4`, `power5`, `power6`, `power7`, "
            "`latency`, `talentGroupsCount`, `activeTalentGroup`, `exploredZones`, "
            "`equipmentCache`, `ammoId`, `knownTitles`, `actionBars`, `grantableLevels`, "
            "`innTriggerId`, `extraBonusTalentCount`) VALUES ("
            f"{guid}, {account_id}, '{name}', {race}, {cls}, {gender}, {level}, 0, 0, "
            f"{skin}, {face}, {hair}, {hcolor}, {facial}, 0, 0, 0, {map_id}, 0, 0, "
            f"{x}, {y}, {z}, 0, 0, 0, 0, 0, 0, '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "
            f"{zone}, 0, '', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, "
            "1, 0, '', '', 0, 0, 0, 0, 0, 0);"
        )

        pb_roster.append(
            "INSERT INTO `citizen_roster` (`guid`, `account_id`, `character_name`, `race`, "
            "`class`, `gender`, `level`, `home_key`, `role`, `poi_id`, `login_priority`, "
            "`enabled`) VALUES ("
            f"{guid}, {account_id}, '{name}', {race}, {cls}, {gender}, {level}, "
            f"'{home}', {role}, {poi}, {prio}, 1);"
        )

        guid += 1

    auth_body = "\n".join(auth_lines) + "\n"
    (OUT_AUTH / "2026_07_15_00_citybot_accounts.sql").write_text(auth_body, encoding="utf-8")
    (OUT_AUTH / "2026_07_15_02_scale_stage_cast_400.sql").write_text(
        "-- mod-city-bots V2 — scale stage cast to 400 (apply if you still have 92-char install)\n"
        "-- Password for every account: citybot_stage\n"
        + auth_body.split("\n", 2)[2],
        encoding="utf-8",
    )

    char_body = "\n".join(char_lines) + "\n"
    (OUT_CHARS / "2026_07_15_01_citybot_characters.sql").write_text(char_body, encoding="utf-8")
    (OUT_CHARS / "2026_07_15_02_scale_stage_cast_400.sql").write_text(
        "-- mod-city-bots V2 — scale stage cast to 400 characters (GUIDs 9000001-9000400)\n"
        + char_body.split("\n", 1)[1],
        encoding="utf-8",
    )

    pb_body = "\n".join(pb_roster + pb_accounts) + "\n"
    (OUT_PB / "2026_07_15_00_citizen_roster.sql").write_text(pb_body, encoding="utf-8")
    (OUT_PB / "2026_07_15_02_scale_stage_cast_400.sql").write_text(
        "-- mod-city-bots V2 — scale stage cast to 400 roster entries\n"
        + pb_body.split("\n", 1)[1],
        encoding="utf-8",
    )

    print(f"Generated {len(cast)} cast members")
    print(f"  Accounts: {ACCOUNT_ID_START}-{ACCOUNT_ID_START + ACCOUNT_COUNT - 1}")
    print(f"  GUIDs:    {GUID_START}-{GUID_START + len(cast) - 1}")
    print(f"  Auth:     {OUT_AUTH / '2026_07_15_00_citybot_accounts.sql'}")
    print(f"  Chars:    {OUT_CHARS / '2026_07_15_01_citybot_characters.sql'}")
    print(f"  Roster:   {OUT_PB / '2026_07_15_00_citizen_roster.sql'}")


def write_spawn_fix_sql():
    """Emit UPDATE-only SQL for live DBs that already applied the broken 400-char seed."""
    cast = cast_entries()
    OUT_CHARS.mkdir(parents=True, exist_ok=True)
    OUT_PB.mkdir(parents=True, exist_ok=True)

    char_lines = [
        "-- mod-city-bots — fix stage cast spawn coords (Org gate in Goldshire, inn beds)",
        "-- Apply to acore_characters + acore_playerbots if 2026_07_15_02 was already imported.",
        "",
    ]
    roster_lines = [
        "-- citizen_roster poi_id corrections for dedicated Goldshire + gate duelists",
        "",
    ]

    guid = GUID_START
    for row in cast:
        name, race, cls, gender, level, home, role, poi, prio = row
        x, y, z, map_id, zone = spawn_for_poi(poi, home, guid)
        char_lines.append(
            f"UPDATE `characters` SET `map` = {map_id}, `zone` = {zone}, "
            f"`position_x` = {x}, `position_y` = {y}, `position_z` = {z} "
            f"WHERE `guid` = {guid};"
        )
        roster_lines.append(
            f"UPDATE `citizen_roster` SET `poi_id` = {poi}, `role` = {role}, "
            f"`home_key` = '{home}' WHERE `guid` = {guid};"
        )
        guid += 1

    path_chars = OUT_CHARS / "2026_07_15_03_fix_stage_cast_spawns.sql"
    path_pb = OUT_PB / "2026_07_15_03_fix_stage_cast_spawns.sql"
    path_chars.write_text("\n".join(char_lines) + "\n", encoding="utf-8")
    path_pb.write_text("\n".join(roster_lines) + "\n", encoding="utf-8")
    print(f"Wrote spawn fix: {path_chars}")
    print(f"Wrote roster fix: {path_pb}")


if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1 and sys.argv[1] == "--spawn-fix":
        write_spawn_fix_sql()
    else:
        generate()
        write_spawn_fix_sql()
