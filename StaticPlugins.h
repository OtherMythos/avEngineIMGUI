//Setup static plugins
#pragma once
#include "native/plugin/src/AvImguiPlugin.h"

void registerStaticPlugins(){
    REGISTER_PLUGIN("AvImguiPlugin", AVImgui::AvImguiPlugin)
}
