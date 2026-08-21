#include "CityBotsRuntime.h"

namespace
{
    bool g_contextsRegistered = false;
}

namespace CityBotsRuntime
{
    bool ContextsRegistered()
    {
        return g_contextsRegistered;
    }

    void MarkContextsRegistered()
    {
        g_contextsRegistered = true;
    }
}
