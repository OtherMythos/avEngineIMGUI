#pragma once

#include "imgui.h"

#include <SDL.h>

namespace AVImgui{

    /**
    Feeds SDL input events into Dear ImGui.

    The engine does not forward raw SDL events (in particular text input) to
    plugins, so an SDL event watch is installed to observe the event stream as
    the engine pumps it. Events are translated into the imgui io event queue.
    */
    class ImguiInput{
    public:
        ImguiInput() = delete;

        //Install the SDL event watch. Call after the imgui context exists.
        static void initialise();
        static void shutdown();

        /**
        Ensure SDL text input is active while imgui wants text input.
        The engine enables/disables SDL text input based on its own gui focus,
        so this is re-checked at the start of each imgui frame.
        */
        static void applyTextInputState();

    private:
        static int SDLCALL eventWatch(void* userdata, SDL_Event* event);
        static ImGuiKey keyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode);
        static void updateKeyModifiers(SDL_Keymod sdlKeyMods);
    };
}
