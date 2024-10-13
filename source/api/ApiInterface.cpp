#include "ApiInterface.h"
#include "vk/ApiVulkan.h"
#include "none/ApiNone.h"

void ApiInterface::InitializeApi(UnityGfxRenderer renderer)
{
	switch (renderer)
	{
	case (kUnityGfxRendererVulkan):
		api = (ApiInterface*)(new ApiVulkan());
		break;
	default:
		api = (ApiInterface*)(new ApiNone());
		break;
	}
}