#define VK_NO_PROTOTYPES
#include <string>
#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsVulkan.h"
#include "ApiVulkan.h"
#include "vulkan/vulkan.h"
#include "UnityGlobals.h"

// vulkan function pointer initialization
#include "VkFunctionList.inl"

static PFN_vkGetInstanceProcAddr UNITY_INTERFACE_API InterceptVulkanInitialization(PFN_vkGetInstanceProcAddr getInstanceProcAddr, void*);
static void LoadVulkanInstanceFunctions(PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkInstance instance);
static void LoadVulkanDeviceFunctions(PFN_vkGetDeviceProcAddr getDeviceProcAddr, VkDevice device);
static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL Hook_vkGetInstanceProcAddr(VkInstance device, const char* funcName);
static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);



static IUnityGraphicsVulkanV2* unityVkInterface;
static UnityVulkanInstance unityVkInstance;

void ApiVulkan::HookInstanceCreation(IUnityInterfaces* unityInterfaces) 
{
    if (IUnityGraphicsVulkanV2* vulkanInterface = unityInterfaces->Get<IUnityGraphicsVulkanV2>())
    {
        vulkanInterface->AddInterceptInitialization(InterceptVulkanInitialization, NULL, 0);
    }
}

static float anisoMipBiasLUT[9] = { 0.f, 0.f, -0.16667f, -0.16667f, -0.2125f, -0.2125f, -0.2125f, -0.2125f, -0.25f};

void ApiVulkan::GfxEventInit() 
{

    unityVkInterface = UnityGlobals::interfaces->Get<IUnityGraphicsVulkanV2>();
    unityVkInstance = unityVkInterface->Instance();

    // For safety, we need to call these again to make sure that all vulkan function pointers are assigned.
    // This is an editor issue, if the dll is freshly installed or the library folder is purged unity 
    // doesn't load the dll in time to call HookInstanceCreation, so all the pointers will be NULL
    // The plugin will be broken until the editor is reloaded, but this prevents a crash
    if (vkGetInstanceProcAddr == nullptr || vkGetDeviceProcAddr == nullptr)
    {
        UNITY_LOG_ERROR(UnityGlobals::log, 
            "Basis Native Rendering Plugin: vkGetInstance/DeviceProcAddr were null, this plugin will not function properly. \
This is probably due to the plugin not being in the asset database at time of editor launch. Restarting should fix this issue.");
        LoadVulkanInstanceFunctions(unityVkInstance.getInstanceProcAddr, unityVkInstance.instance);
        LoadVulkanDeviceFunctions(vkGetDeviceProcAddr, unityVkInstance.device);
    }

    VkPhysicalDeviceProperties props = {};
    vkGetPhysicalDeviceProperties(unityVkInstance.physicalDevice, &props);
    // Only NV needs the mip bias (0x10DE is nvidia's ID)
    if (props.vendorID != 0x10DE)
    {
        constexpr int lutCount = std::size(anisoMipBiasLUT);
        for (int i = 0; i < lutCount; i++)
        {
            anisoMipBiasLUT[i] = 0.f;
        }
    }
    unityVkInterface->InterceptVulkanAPI("vkCreateSampler", (PFN_vkVoidFunction)Hook_vkCreateSampler);
}

void ApiVulkan::GfxEventShutdown() 
{
// Nothing to do
}


static PFN_vkGetInstanceProcAddr UNITY_INTERFACE_API InterceptVulkanInitialization(PFN_vkGetInstanceProcAddr getInstanceProcAddr, void*)
{
    vkGetInstanceProcAddr = getInstanceProcAddr;
    return Hook_vkGetInstanceProcAddr;
}

static void LoadVulkanInstanceFunctions(PFN_vkGetInstanceProcAddr getInstanceProcAddr, VkInstance instance)
{
    // Macro to get the function pointers for each instance level function listed in VkFunctionList using vkGetInstanceProcAddr
#define LOAD_VULKAN_INSTANCE_FUNC(fn) if (!fn) fn = (PFN_##fn)getInstanceProcAddr(instance, #fn)
    VULKAN_INSTANCE_FUNCTIONS(LOAD_VULKAN_INSTANCE_FUNC);
#undef LOAD_VULKAN_INSTANCE_FUNC
}

static void LoadVulkanDeviceFunctions(PFN_vkGetDeviceProcAddr getDeviceProcAddr, VkDevice device)
{
    // Macro to get the function pointers for each device level function listed in VkFunctionList using vkGetInstanceProcAddr
#define LOAD_VULKAN_DEVICE_FUNC(fn) if (!fn) fn = (PFN_##fn)getDeviceProcAddr(device, #fn)
    VULKAN_DEVICE_FUNCTIONS(LOAD_VULKAN_DEVICE_FUNC);
#undef LOAD_VULKAN_DEVICE_FUNC
}



static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL Hook_vkGetInstanceProcAddr(VkInstance device, const char* funcName)
{
    if (!funcName)
        return NULL;

#define INTERCEPT(fn) if (strcmp(funcName, #fn) == 0) return (PFN_vkVoidFunction)&Hook_##fn

#ifndef DUMMY_PLUGIN
    INTERCEPT(vkCreateInstance);
    INTERCEPT(vkCreateDevice);
#endif


#undef INTERCEPT

    return vkGetInstanceProcAddr(device, funcName);
}

static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance)
{
    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");

    VkResult result = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result == VK_SUCCESS)
    {
        LoadVulkanInstanceFunctions(vkGetInstanceProcAddr, *pInstance);
    }

    return result;
}

static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice)
{
    VkResult result = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result == VK_SUCCESS)
    {
        LoadVulkanDeviceFunctions(vkGetDeviceProcAddr, *pDevice);
    }
    return result;
}


static VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler)
{
    VkSamplerCreateInfo* eCreateInfo = const_cast<VkSamplerCreateInfo*>(pCreateInfo);
    if (eCreateInfo->anisotropyEnable)
    {
        constexpr int lutCount = std::size(anisoMipBiasLUT);
        int index = (int)fmin(fmax(0, eCreateInfo->maxAnisotropy), lutCount - 1);
        float anisoMipBias = anisoMipBiasLUT[index];
        eCreateInfo->mipLodBias = eCreateInfo->mipLodBias + anisoMipBias;
        eCreateInfo->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
    return vkCreateSampler(device, eCreateInfo, pAllocator, pSampler);
}