#pragma once

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityLog.h"

class UnityGlobals
{
public:
    static inline IUnityInterfaces* interfaces		= nullptr;
    static inline IUnityGraphics*   graphics	    = nullptr;
    static inline IUnityLog*        log			    = nullptr;
    static inline UnityGfxRenderer  renderer        = kUnityGfxRendererNull;
};