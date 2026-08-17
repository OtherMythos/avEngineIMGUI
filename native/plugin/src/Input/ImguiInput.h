#pragma once

#include "imgui.h"

namespace AVImgui{

    /**
    Feeds engine input into Dear ImGui.

    The engine statically links SDL and does not re-export its symbols, so a
    plugin cannot call SDL directly (it fails to load on Linux). Instead this
    reads the engine's InputManager and translates its input into imgui's io.
    Mouse button transitions arrive through an InputManager listener so a
    complete click between two frames remains ordered; other input is polled
    once per imgui frame. Only the SDL *scancode* enum is used, which is a
    compile-time constant and needs no SDL runtime symbol.

    Consequences of polling the remaining input rather than SDL's event stream:
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
        static void mouseButtonEvent(int mouseButton, bool pressed, void* userData);
        static ImGuiKey scancodeToImGuiKey(int scancode);
    };
}
