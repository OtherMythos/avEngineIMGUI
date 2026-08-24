#pragma once

#include "imgui.h"

namespace AVImgui{

    /**
    Feeds engine input into Dear ImGui, and tells the engine when imgui has
    swallowed it.

    This registers an input layer with the engine's InputRouter at the overlay
    priority, so every event is offered to imgui before the gui system or the
    game sees it. When imgui reports that it wants the mouse or the keyboard,
    the layer consumes the event and nothing below is told about it at all.
    That is what stops a click on a debug window also driving the game beneath.

    Input arrives as events rather than being polled from the InputManager,
    which it has to: a press imgui consumes never reaches the InputManager, so
    polling it would leave imgui unable to see its own clicks.

    The engine statically links SDL and does not re-export its symbols, so a
    plugin cannot call SDL directly (it fails to load on Linux). Only the SDL
    *scancode* enum is used, which is a compile-time constant and needs no SDL
    runtime symbol.
    */
    class ImguiInput{
    public:
        ImguiInput() = delete;

        static void initialise();
        static void shutdown();

    private:
        //Whether imgui is drawing, and so whether its capture flags mean
        //anything. A layer which is not live is skipped entirely.
        static bool isLive(void* userData);

        static bool wantsPointer(void* userData);
        static bool wantsKeyboard(void* userData);
        static bool wantsTextInput(void* userData);

        static void onMouseMove(float x, float y, float normX, float normY, bool focused, void* userData);
        static bool onMouseButton(int button, bool pressed, void* userData);
        static bool onMouseWheel(float x, float y, void* userData);
        static bool onKey(int scancode, int keycode, int keyMod, bool pressed, void* userData);
        static bool onTextInput(const char* text, void* userData);
        static void onInputCancelled(void* userData);

        static ImGuiKey scancodeToImGuiKey(int scancode);
    };
}
