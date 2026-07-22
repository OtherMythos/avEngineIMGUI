#pragma once

#include <squirrel.h>

namespace AVImgui{

    /**
    Exposes the Dear ImGui api to squirrel as the _imgui namespace.

    Scripts build the gui each frame from an update callback. The imgui frame
    is begun implicitly by the first _imgui call of each engine frame, and
    rendered by the imgui compositor pass.
    */
    class ImguiNamespace{
    public:
        ImguiNamespace() = delete;

        static void setupNamespace(HSQUIRRELVM vm);

    private:
        //Registers the ImGui enum values as integer constants (implemented in ImguiConstants.cpp).
        static void _setupConstants(HSQUIRRELVM vm);
    };
}
