#pragma once

//Version of the plugin. Bump by hand on release.
//The git hash that goes with it is generated at build time into
//AvImguiGitVersion.h (see cmake/AvImguiGitVersion.h.in).

#define AV_IMGUI_PLUGIN_VERSION_MAJOR 0
#define AV_IMGUI_PLUGIN_VERSION_MINOR 1
#define AV_IMGUI_PLUGIN_VERSION_PATCH 0
#define AV_IMGUI_PLUGIN_VERSION_SUFFIX "unstable"

#define AV_IMGUI_PLUGIN_STRINGIFY_(x) #x
#define AV_IMGUI_PLUGIN_STRINGIFY(x) AV_IMGUI_PLUGIN_STRINGIFY_(x)

#define AV_IMGUI_PLUGIN_VERSION_STRING          \
    AV_IMGUI_PLUGIN_STRINGIFY(AV_IMGUI_PLUGIN_VERSION_MAJOR) "." \
    AV_IMGUI_PLUGIN_STRINGIFY(AV_IMGUI_PLUGIN_VERSION_MINOR) "." \
    AV_IMGUI_PLUGIN_STRINGIFY(AV_IMGUI_PLUGIN_VERSION_PATCH)
