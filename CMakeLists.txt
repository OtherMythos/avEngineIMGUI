if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message( FATAL_ERROR "This CMakeLists.txt should be run as a subdirectory to register static plugins for an engine build. Native code can be built using the CMake project found in the native/ directory.")
endif()

message("Processing avEngineIMGUI cmake files")

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

set(StaticPluginHeaderIncludes ${CMAKE_CURRENT_LIST_DIR})
set(StaticPluginSrcIncludes ${CMAKE_CURRENT_LIST_DIR}/plugins/include)

set(BuildDir build)
set(StaticEnding "android")
if(${CMAKE_SYSTEM_NAME} STREQUAL "iOS")
    set(BuildDir buildIos)
    set(IosBuildEnding "-iphoneos")
    set(StaticEnding "ios")
endif()

FIND_LIBRARY(AV_IMGUI_PLUGIN NAMES AvImguiPlugin_static AvImguiPlugin_${StaticEnding}_static_${CMAKE_BUILD_TYPE} AvImguiPlugin_${StaticEnding}_static_Release
    PATHS ${CMAKE_CURRENT_LIST_DIR}/native/${BuildDir}/plugin/${CMAKE_BUILD_TYPE}${IosBuildEnding}
    ${CMAKE_CURRENT_LIST_DIR}/native/buildAndroid/plugin
    ${CMAKE_CURRENT_LIST_DIR}/plugins/
    PATH_SUFFIXES a lib
    )

if(AV_IMGUI_PLUGIN)
    set(StaticPluginLibraries "${AV_IMGUI_PLUGIN}")
    set(StaticPluginLibraries "${StaticPluginLibraries}" PARENT_SCOPE)

    set(StaticPluginIncludes "${StaticPluginHeaderIncludes};${StaticPluginSrcIncludes}")
    set(StaticPluginIncludes ${StaticPluginIncludes} PARENT_SCOPE)

    MESSAGE(STATUS "Found AvImguiPlugin: ${StaticPluginLibraries}")
    MESSAGE(STATUS "Found AvImguiPlugin includes: ${StaticPluginIncludes}")
else()
    message( FATAL_ERROR "Unable to find the AvImguiPlugin static library")
endif()

#Nothing to bundle for a plugin-only project.
set(PROJ_TO_BUNDLE "" CACHE INTERNAL "PROJ_TO_BUNDLE")
