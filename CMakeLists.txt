cmake_minimum_required(VERSION 3.25)

if(${CMAKE_VERSION} VERSION_LESS 3.26)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.26)
endif()

project(BasisNativeGfx 
		VERSION 1.0.0
        DESCRIPTION "Unity native graphics plugin for Basis"
        LANGUAGES CXX)		

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") AND (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
	# CMake >3.28 tries to set the include flag to -imsvc for Clang-CL, which doesn't work when opening the project in visual studio. 
	#set(CMAKE_INCLUDE_SYSTEM_FLAG_C "/I")
	unset(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
endif()

find_package(Vulkan REQUIRED)
if(NOT DEFINED ENV{UNITY_PLUGIN_DIR})
	message( SEND_ERROR "You need to set the environment variable UNITY_PLUGIN_DIR to the location of the PluginAPI folder in your unity install ('unity version number'/Editor/Data/PluginAPI/)" )
endif()

set(UNITY_INCLUDE_DIR "$ENV{UNITY_PLUGIN_DIR}")

set(SRC_PATH "${CMAKE_SOURCE_DIR}/source")


#NOTE: unity has magic naming for graphics plugins, must begin with "GfxPlugin" to be automatically preloaded
# This may not be necessary, as one can also specify to preload the plugin in the import settings?
add_library(GfxPluginBasis SHARED 
"${SRC_PATH}/libMain.cpp"
"${SRC_PATH}/api/ApiInterface.cpp"
"${SRC_PATH}/api/vk/ApiVulkan.cpp"
)

target_include_directories(GfxPluginBasis PUBLIC "${SRC_PATH}" "${UNITY_INCLUDE_DIR}")
target_link_libraries(GfxPluginBasis Vulkan::Vulkan)


