#ifndef MOD_CITY_BOTS_CB_CORE_COMPAT_H
#define MOD_CITY_BOTS_CB_CORE_COMPAT_H

// Keeps mod-city-bots building on both generations of the mod-playerbots
// AzerothCore fork:
//  - older cores name the bot-session test WorldSession::IsBot();
//  - newer cores (AzerothCore #27533) renamed it WorldSession::IsHeadless().
// CbIsBotSession() picks whichever the core has, at compile time.
//  - newer mod-playerbots renamed the PlayerbotAIConfig settings to PascalCase
//    (randomBotAutologin -> RandomBotAutologin, ...); CbCfg*() reads whichever
//    spelling exists.
//
// PlayerbotsDatabase moved out of the core's DatabaseEnv.h into mod-playerbots
// (mod-playerbots #2793), so its header has to be included by name where it is
// used. Cores that still declare it in DatabaseEnv.h do not need the include.

#include "WorldSession.h"

#if defined(__has_include)
#if __has_include("PlayerbotsDatabase.h")
#include "PlayerbotsDatabase.h"
#endif
#endif

namespace CbCoreCompat
{
    // Preferred overload (int -> exact match): the newer core's IsHeadless().
    template <typename S>
    auto IsBotSession(S const* session, int) -> decltype(session->IsHeadless())
    {
        return session->IsHeadless();
    }

    // Fallback (int -> long conversion, so only picked when IsHeadless() is
    // missing): the older core's IsBot().
    template <typename S>
    auto IsBotSession(S const* session, long) -> decltype(session->IsBot())
    {
        return session->IsBot();
    }
}

// True when the session belongs to a bot (a session without a real client).
// Callers keep their own null check on the session.
inline bool CbIsBotSession(WorldSession const* session)
{
    return CbCoreCompat::IsBotSession(session, 0);
}


// PlayerbotAIConfig settings under both spellings. Pass sPlayerbotAIConfig.
namespace CbCoreCompat
{
#define CB_CFG_READER(Fn, NewName, OldName)                                     \
    template <typename C>                                                       \
    auto Fn(C const& cfg, int) -> decltype(cfg.NewName) { return cfg.NewName; } \
    template <typename C>                                                       \
    auto Fn(C const& cfg, long) -> decltype(cfg.OldName) { return cfg.OldName; }

    CB_CFG_READER(Enabled, Enabled, enabled)
    CB_CFG_READER(RandomBotAutologin, RandomBotAutologin, randomBotAutologin)
    CB_CFG_READER(SightDistance, SightDistance, sightDistance)
    CB_CFG_READER(EnableRandomBotTrading, EnableRandomBotTrading, enableRandomBotTrading)

#undef CB_CFG_READER
}

template <typename C>
inline auto CbCfgEnabled(C const& cfg) { return CbCoreCompat::Enabled(cfg, 0); }
template <typename C>
inline auto CbCfgRandomBotAutologin(C const& cfg) { return CbCoreCompat::RandomBotAutologin(cfg, 0); }
template <typename C>
inline auto CbCfgSightDistance(C const& cfg) { return CbCoreCompat::SightDistance(cfg, 0); }
template <typename C>
inline auto CbCfgEnableRandomBotTrading(C const& cfg) { return CbCoreCompat::EnableRandomBotTrading(cfg, 0); }

#endif
