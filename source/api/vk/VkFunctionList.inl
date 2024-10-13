
// Generate function pointers for all used vulkan functions. Add any needed functions here

// Instance level functions
#define VULKAN_INSTANCE_FUNCTIONS(macro) \
    macro(vkCreateInstance); \
    macro(vkCreateDevice); \
    macro(vkGetDeviceProcAddr); \
    macro(vkGetPhysicalDeviceFeatures2); \
    macro(vkGetPhysicalDeviceProperties); \
    macro(vkGetPhysicalDeviceProperties2);

// Device Level Functions
#define VULKAN_DEVICE_FUNCTIONS(macro) \
    macro(vkCreateSampler);


#define VULKAN_DEFINE_FUNCPTR(func) static PFN_##func func


VULKAN_DEFINE_FUNCPTR(vkGetInstanceProcAddr);
VULKAN_INSTANCE_FUNCTIONS(VULKAN_DEFINE_FUNCPTR)
VULKAN_DEVICE_FUNCTIONS(VULKAN_DEFINE_FUNCPTR)