#AvImguiPlugin — static plugin integration for avEngine projects.
#
#This file ships inside the plugin distribution. Drop the distribution into a
#project as its plugins/ directory, then from the project's own CMakeLists.txt
#(the one the engine loads through AV_PROJECT_DIR):
#
#    list(APPEND StaticPluginIncludes ${CMAKE_CURRENT_LIST_DIR})
#    include(${CMAKE_CURRENT_LIST_DIR}/plugins/AvImguiPlugin.cmake)
#
#and register the plugin in the project's StaticPlugins.h:
#
#    #include "AvImguiPlugin.h"
#
#    void registerStaticPlugins(){
#        REGISTER_PLUGIN("AvImguiPlugin", AVImgui::AvImguiPlugin)
#    }
#
#Then build the engine with -DUSE_STATIC_PLUGINS=True -DAV_PROJECT_DIR=<project>.
#
#Only static platforms (iOS, Android) need any of this. Desktop builds load the
#shared library sitting next to this file at runtime, so they need nothing more
#than the "Plugins" entry in avSetup.cfg.

set(AvImguiPlugin_DIR ${CMAKE_CURRENT_LIST_DIR})

if(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(AvImguiPlugin_PLATFORM "ios")
else()
    set(AvImguiPlugin_PLATFORM "android")
endif()

#find_library supplies the lib prefix and .a suffix.
find_library(AvImguiPlugin_LIBRARY
    NAMES
        AvImguiPlugin_${AvImguiPlugin_PLATFORM}_static_${CMAKE_BUILD_TYPE}
        AvImguiPlugin_${AvImguiPlugin_PLATFORM}_static_Release
        AvImguiPlugin_static
    PATHS ${AvImguiPlugin_DIR}
    NO_DEFAULT_PATH
)

if(NOT AvImguiPlugin_LIBRARY)
    message(FATAL_ERROR
        "AvImguiPlugin: no ${AvImguiPlugin_PLATFORM} static library for build type "
        "'${CMAKE_BUILD_TYPE}' in ${AvImguiPlugin_DIR}.\n"
        "The distribution for this platform needs to be extracted there "
        "(expected libAvImguiPlugin_${AvImguiPlugin_PLATFORM}_static_${CMAKE_BUILD_TYPE}.a).")
endif()

list(APPEND StaticPluginLibraries ${AvImguiPlugin_LIBRARY})
list(APPEND StaticPluginIncludes ${AvImguiPlugin_DIR}/include)

#include() shares the caller's scope, so this hands the project's whole list -
#its own entries included - up to the engine build.
set(StaticPluginLibraries "${StaticPluginLibraries}" PARENT_SCOPE)
set(StaticPluginIncludes "${StaticPluginIncludes}" PARENT_SCOPE)

message(STATUS "AvImguiPlugin: ${AvImguiPlugin_LIBRARY}")
