#ifndef MOD_CITYBOTS_CBCITIZENACCOUNTMGR_H
#define MOD_CITYBOTS_CBCITIZENACCOUNTMGR_H

#include "Define.h"

namespace CbCitizenAccountMgr
{
    // V2: accounts + characters come from shipped SQL (fixed IDs/GUIDs).
    void ValidateStageCast();
    bool IsCitizenAccount(uint32 accountId);
    bool IsStageCastAccount(uint32 accountId);
    uint32 CountCitizenCharacters();
}

#endif
