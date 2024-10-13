#pragma once
#include "IUnityGraphics.h"

class ApiInterface
{
public:
	static inline ApiInterface* api = nullptr;
	static void InitializeApi(UnityGfxRenderer renderer);

	virtual void GfxEventInit() {}
	virtual void GfxEventShutdown() {}
	virtual ~ApiInterface() {}
};