#ifndef MOD_CITYBOTS_CBSTRATEGYGATE_H
#define MOD_CITYBOTS_CBSTRATEGYGATE_H

#include "Define.h"

class Player;

namespace CbStrategyGate
{
    // Re-arm the one-shot login staging for a roster citizen (login hook).
    void ResetLoginStaging(uint32 guidLow);
    enum class Action
    {
        None,
        Install,
        Strip
    };

    constexpr Action Decide(bool shouldBeCitizen, bool hasStrategy)
    {
        if (shouldBeCitizen && !hasStrategy)
            return Action::Install;
        if (!shouldBeCitizen && hasStrategy)
            return Action::Strip;
        return Action::None;
    }

    bool IsEligibleCitizen(Player* bot);
    void Reconcile(Player* bot);
    void ReconcileAllBots();
    void ReconcileRosterBots();
}

#endif
