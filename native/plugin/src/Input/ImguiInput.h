#pragma once

#include "imgui.h"

namespace AVImgui{

    /**
    Feeds engine input into Dear ImGui.

    The engine statically links SDL and does not re-export its symbols, so a
    plugin cannot call SDL directly (it fails to load on Linux). Instead this
    polls the engine's InputManager once per imgui frame and translates the
    state into imgui's io. Only the SDL *scancode* enum is used, which is a
    compile-time constant and needs no SDL runtime symbol.

    Consequences of polling rather than watching the event stream:
    - Mouse (position, buttons, wheel) and keyboard keys work fully.
    - Text input is reconstructed from key state for a US keyboard layout, so
      typing into an imgui text field works for ASCII but does not follow other
      layouts or the OS input method.
    */
    class ImguiInput{
    public:
        ImguiInput() = delete;

        static void initialise();
        static void shutdown();

        //Poll the engine input and push it into imgui. Called once per imgui
        //frame, before ImGui::NewFrame().
        static void update();

    private:
        static ImGuiKey scancodeToImGuiKey(int scancode);
    };
}
