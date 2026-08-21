#ifndef MOD_CITYBOTS_CBSETTINGSREGISTRY_H
#define MOD_CITYBOTS_CBSETTINGSREGISTRY_H

#include <cstddef>

enum class CbType
{
    Bool,
    UInt,
    Int,
    Float
};

struct CbSettingDef
{
    char const* key;
    CbType      type;
    double      defVal;
    double      minVal;
    double      maxVal;
};

inline constexpr CbSettingDef kCbSettings[] =
{
    { "Enable",                  CbType::Bool,  1,   0,   1   },
    { "LogLevel",                CbType::UInt,  1,   0,   3   },
    { "LogMirrorToModule",       CbType::Bool,  0,   0,   1   },
    { "Percent",                 CbType::UInt,  0,   0,  100  },
    { "UseDedicatedAccounts",    CbType::Bool,  1,   0,   1   },
    { "DedicatedCount",          CbType::UInt,  0,   0, 5000  },
    { "HybridPopulation",        CbType::Bool,  0,   0,   1   },
    { "CountRealPlayers",        CbType::Bool,  1,   0,   1   },
    { "BurstGraceSec",           CbType::UInt,  180, 0, 3600  },
    { "ActivityDurationMinSec",  CbType::UInt,  45,  30, 3600 },
    { "ActivityDurationMaxSec",  CbType::UInt,  180, 60, 7200 },
    { "Goldshire.MinInnDancers", CbType::UInt,  2,   0,  20   },
    { "Goldshire.PlazaTalkers",  CbType::UInt,  4,   0,  50   },
    { "Goldshire.MinDuelers",   CbType::UInt,  2,   0,  20   },
    { "RandomSayChance",         CbType::UInt,  0,   0,  100  },
    { "EnableSay",               CbType::Bool,  0,   0,   1   },
    { "SayCooldownSec",          CbType::UInt,  30,  5, 3600  },
    { "EnableFishing",           CbType::Bool,  1,   0,   1   },
    { "EnableLogoutCycle",       CbType::Bool,  0,   0,   1   },
    { "TeleportIfOutsideCity",   CbType::Bool,  1,   0,   1   },
    { "KeepEmoteStrategy",       CbType::Bool,  1,   0,   1   },
    { "KeepFoodStrategy",        CbType::Bool,  1,   0,   1   },
    { "MountBetweenPoiChance",   CbType::UInt,  90,  0, 100   },
    { "Weight.WanderDistrict",   CbType::UInt,  30,  0, 1000  },
    { "Weight.SitInTavern",      CbType::UInt,  20,  0, 1000  },
    { "Weight.RandomEmote",      CbType::UInt,  15,  0, 1000  },
    { "Weight.VisitAuctionHouse", CbType::UInt, 10,  0, 1000  },
    { "Weight.VisitMailbox",     CbType::UInt,  10,  0, 1000  },
    { "Weight.BuyFood",          CbType::UInt,  10,  0, 1000  },
    { "Weight.VisitTrainer",     CbType::UInt,  8,   0, 1000  },
    { "Weight.WatchDuels",       CbType::UInt,  12,  0, 1000  },
    { "Weight.DrinkInTavern",    CbType::UInt,  10,  0, 1000  },
    { "Weight.WalkCityWalls",    CbType::UInt,  10,  0, 1000  },
    { "Weight.FishAtDocks",      CbType::UInt,  5,   0, 1000  },
    { "Weight.ReturnToInn",      CbType::UInt,  15,  0, 1000  },
    { "Weight.Logout",           CbType::UInt,  3,   0, 1000  },
    { "EnableCrowdScenes",       CbType::Bool,  1,   0,   1   },
    { "CrowdPlayerRadius",       CbType::UInt,  60,  10,  200 },
    { "CrowdChatCooldownSec",    CbType::UInt,  90,  10, 3600 },
    { "CrowdAfkMinSec",          CbType::UInt,  60,  10,  600 },
    { "CrowdAfkMaxSec",          CbType::UInt,  180, 10, 1200 },
};

inline constexpr std::size_t kCbSettingCount = sizeof(kCbSettings) / sizeof(kCbSettings[0]);

#endif
