#ifndef MOD_CITYBOTS_CBCITIZENLOGINMGR_H
#define MOD_CITYBOTS_CBCITIZENLOGINMGR_H

#include "Define.h"

namespace CbCitizenLoginMgr
{
    void PrepareDedicatedPool();
    void RestoreCitizensToCurrentBots();
    void Tick(uint32 diff);
    void OnRosterBotLoggedIn(uint32 guidLow);
    uint32 CountOnlineCitizens();
    bool UsesDedicatedPool();
}

#endif
