#include "Ai/City/CityBots/Settings/CbSettings.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <string>

#include "Config.h"

namespace
{
    std::string FullKey(char const* keySuffix)
    {
        return std::string("CitizenBots.") + keySuffix;
    }

    double ConfValueUncached(CbSettingDef const& d)
    {
        std::string const full = FullKey(d.key);
        switch (d.type)
        {
            case CbType::Bool:
                return sConfigMgr->GetOption<bool>(full, d.defVal != 0.0, false) ? 1.0 : 0.0;
            case CbType::UInt:
                return static_cast<double>(
                    sConfigMgr->GetOption<uint32>(full, static_cast<uint32>(d.defVal), false));
            case CbType::Int:
                return static_cast<double>(
                    sConfigMgr->GetOption<int32>(full, static_cast<int32>(d.defVal), false));
            case CbType::Float:
            default:
                return static_cast<double>(
                    sConfigMgr->GetOption<float>(full, static_cast<float>(d.defVal), false));
        }
    }

    std::array<std::atomic<double>, kCbSettingCount> g_confCache;
    std::atomic<bool> g_confCacheReady{false};
    std::mutex g_confCacheLoadMutex;

    void PopulateConfCache()
    {
        for (std::size_t i = 0; i < kCbSettingCount; ++i)
            g_confCache[i].store(ConfValueUncached(kCbSettings[i]), std::memory_order_relaxed);
        g_confCacheReady.store(true, std::memory_order_release);
    }

    CbSettingDef const* FindDef(char const* keySuffix)
    {
        for (std::size_t i = 0; i < kCbSettingCount; ++i)
            if (strcmp(kCbSettings[i].key, keySuffix) == 0)
                return &kCbSettings[i];
        return nullptr;
    }

    double GetRaw(char const* keySuffix)
    {
        CbSettingDef const* d = FindDef(keySuffix);
        if (!d)
            return 0.0;

        if (!g_confCacheReady.load(std::memory_order_acquire))
        {
            std::lock_guard<std::mutex> lock(g_confCacheLoadMutex);
            if (!g_confCacheReady.load(std::memory_order_relaxed))
                PopulateConfCache();
        }

        for (std::size_t i = 0; i < kCbSettingCount; ++i)
            if (&kCbSettings[i] == d)
                return g_confCache[i].load(std::memory_order_relaxed);

        return ConfValueUncached(*d);
    }
}

namespace CbSettings
{
    void InvalidateConfCache()
    {
        std::lock_guard<std::mutex> lock(g_confCacheLoadMutex);
        g_confCacheReady.store(false, std::memory_order_release);
    }

    bool GetBool(char const* keySuffix)
    {
        return GetRaw(keySuffix) != 0.0;
    }

    uint32 GetUInt(char const* keySuffix)
    {
        if (FindDef(keySuffix))
            return static_cast<uint32>(GetRaw(keySuffix));

        return sConfigMgr->GetOption<uint32>(FullKey(keySuffix), 0u, false);
    }

    int32 GetInt(char const* keySuffix)
    {
        return static_cast<int32>(GetRaw(keySuffix));
    }

    float GetFloat(char const* keySuffix)
    {
        return static_cast<float>(GetRaw(keySuffix));
    }
}
