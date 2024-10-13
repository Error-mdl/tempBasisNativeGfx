#pragma once
#include "api/ApiInterface.h"

class ApiVulkan : ApiInterface
{
public:
	static void HookInstanceCreation(IUnityInterfaces* unityInterfaces);
	void GfxEventInit();
	void GfxEventShutdown();
	~ApiVulkan() {};
};