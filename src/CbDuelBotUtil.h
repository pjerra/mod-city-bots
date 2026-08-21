#ifndef MOD_CITYBOTS_CBDUELBOTUTIL_H
#define MOD_CITYBOTS_CBDUELBOTUTIL_H

#include "Define.h"

class Player;

namespace CbDuelBotUtil
{
    uint8 RequiredLevel();
    bool IsEligibleLevel(uint8 level);
    bool IsEligible(Player* bot);
    void EnsureLevel(Player* bot);
}

#endif
