#ifndef MOD_CITY_BOTS_CB_PLAYERBOT_HOLDER_ACCESS_H
#define MOD_CITY_BOTS_CB_PLAYERBOT_HOLDER_ACCESS_H

#include "ObjectGuid.h"
#include "PlayerbotMgr.h"

class CbPlayerbotHolderAccess : public PlayerbotHolder
{
public:
    static uint32 RemoveFromPlayerbots(PlayerbotHolder& holder, ObjectGuid guid)
    {
        auto& map = reinterpret_cast<CbPlayerbotHolderAccess&>(holder).playerBots;
        return static_cast<uint32>(map.erase(guid));
    }

    static uint32 CountBotLoading()
    {
        return static_cast<uint32>(botLoading.size());
    }

    static bool IsGuidInBotLoading(ObjectGuid guid)
    {
        return botLoading.find(guid) != botLoading.end();
    }

    static void ReleaseBotLoading(ObjectGuid guid)
    {
        botLoading.erase(guid);
    }
};

#endif
