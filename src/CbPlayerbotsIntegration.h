#ifndef MOD_CITY_BOTS_CB_PLAYERBOTS_INTEGRATION_H
#define MOD_CITY_BOTS_CB_PLAYERBOTS_INTEGRATION_H

#include "Define.h"

#include <string>

namespace CbPlayerbotsIntegration
{
    enum class StageCastLoginResult : uint8
    {
        LoggedIn,
        AlreadyOnline,
        AlreadyLoading,
        ZombieLogout,
        NoAccount,
        Failed
    };

    uint32 CountBotLoading();
    bool IsGuidInBotLoading(uint32 guidLow);
    void ReleaseBotLoading(uint32 guidLow);
    uint32 RemoveFromRandomPlayerList(uint32 guidLow);
    uint32 RemoveStageCastFromRandomPlayerList();
    std::size_t CharacterDatabaseQueueSize();
    StageCastLoginResult TryLoginStageCastBot(uint32 guidLow, std::string& detail);
}

#endif
