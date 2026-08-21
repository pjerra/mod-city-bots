#ifndef MOD_CITYBOTS_CBLOG_H
#define MOD_CITYBOTS_CBLOG_H

#include "Ai/City/CityBots/Settings/CbSettings.h"
#include "Log.h"

#include <string>

namespace CbLog
{
    enum Level : uint32
    {
        ErrorLevel = 0,
        WarnLevel = 1,
        InfoLevel = 2,
        DebugLevel = 3
    };

    inline bool ShouldLog(Level level)
    {
        return CbSettings::GetUInt("LogLevel") >= static_cast<uint32>(level);
    }

    inline bool MirrorToModule()
    {
        return CbSettings::GetBool("LogMirrorToModule");
    }

    // Route city-bot messages to Playerbots.log (category "playerbots") so they
    // are visible beside random-bot login noise. Optional mirroring to "module"
    // is disabled by default because many console configs print both categories.
    template <typename... Args>
    void Info(char const* fmt, Args&&... args)
    {
        if (!ShouldLog(InfoLevel))
            return;

        std::string msg = Acore::StringFormat(fmt, std::forward<Args>(args)...);
        LOG_INFO("playerbots", "[mod-city-bots] {}", msg);
        if (MirrorToModule())
            LOG_INFO("module", "mod-city-bots: {}", msg);
    }

    template <typename... Args>
    void Warn(char const* fmt, Args&&... args)
    {
        if (!ShouldLog(WarnLevel))
            return;

        std::string msg = Acore::StringFormat(fmt, std::forward<Args>(args)...);
        LOG_WARN("playerbots", "[mod-city-bots] {}", msg);
        if (MirrorToModule())
            LOG_WARN("module", "mod-city-bots: {}", msg);
    }

    template <typename... Args>
    void Error(char const* fmt, Args&&... args)
    {
        if (!ShouldLog(ErrorLevel))
            return;

        std::string msg = Acore::StringFormat(fmt, std::forward<Args>(args)...);
        LOG_ERROR("playerbots", "[mod-city-bots] {}", msg);
        if (MirrorToModule())
            LOG_ERROR("module", "mod-city-bots: {}", msg);
    }

    template <typename... Args>
    void Debug(char const* fmt, Args&&... args)
    {
        if (!ShouldLog(DebugLevel))
            return;

        std::string msg = Acore::StringFormat(fmt, std::forward<Args>(args)...);
        LOG_DEBUG("playerbots", "[mod-city-bots] {}", msg);
        if (MirrorToModule())
            LOG_DEBUG("module", "mod-city-bots: {}", msg);
    }
}

#endif
