#ifndef MOD_CITYBOTS_CBSETTINGS_H
#define MOD_CITYBOTS_CBSETTINGS_H

#include "Define.h"

#include "Ai/City/CityBots/Settings/CbSettingsRegistry.h"

namespace CbSettings
{
    void InvalidateConfCache();
    bool GetBool(char const* keySuffix);
    uint32 GetUInt(char const* keySuffix);
    int32 GetInt(char const* keySuffix);
    float GetFloat(char const* keySuffix);
}

#endif
