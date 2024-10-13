#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityLog.h"

#include "UnityGlobals.h"
#include "api/ApiInterface.h"
#include "api/vk/ApiVulkan.h"

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces);
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload();
static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	UnityGlobals::interfaces = unityInterfaces;
	
	UnityGlobals::graphics = unityInterfaces->Get<IUnityGraphics>();
	UnityGlobals::graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
	UnityGlobals::log = unityInterfaces->Get<IUnityLog>();

	// Copied from unity's example, not sure why the renderer being null indicates its vulkan
	if (UnityGlobals::graphics->GetRenderer() == kUnityGfxRendererNull)
	{
		ApiVulkan::HookInstanceCreation(unityInterfaces);
	}

	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	if (ApiInterface::api)
	{
		delete (ApiInterface::api);
	}
}

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	switch (eventType)
	{
	case kUnityGfxDeviceEventInitialize:
		UnityGlobals::renderer = UnityGlobals::graphics->GetRenderer();
		ApiInterface::InitializeApi(UnityGlobals::renderer);
		ApiInterface::api->GfxEventInit();
		break;
	case kUnityGfxDeviceEventShutdown:
		if (ApiInterface::api)
		{
			ApiInterface::api->GfxEventShutdown();
		}
		break;
	default:
		break;
	}
}