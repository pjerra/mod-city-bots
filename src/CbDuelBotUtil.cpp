#include "CbDuelBotUtil.h"

#include "Ai/City/CityBots/CitizenInfo.h"

#include "Player.h"

namespace CbDuelBotUtil
{
    uint8 RequiredLevel()
    {
        return CITIZEN_DUEL_HUB_LEVEL;
    }

    bool IsEligibleLevel(uint8 level)
    {
        return level >= RequiredLevel();
    }

    bool IsEligible(Player* bot)
    {
        return bot && IsEligibleLevel(bot->GetLevel());
    }

    void EnsureLevel(Player* bot)
    {
        if (!bot || !bot->IsInWorld() || bot->IsBeingTeleported() || IsEligible(bot))
            return;

        bot->GiveLevel(RequiredLevel());
        bot->SetFullHealth();
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));
    }
}
