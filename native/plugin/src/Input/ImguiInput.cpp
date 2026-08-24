#include "ImguiInput.h"

//Only the scancode enum is pulled in - a compile-time constant table, no SDL
//runtime symbols (the engine statically links SDL and does not re-export it).
#include <SDL_scancode.h>

#include "System/BaseSingleton.h"
#include "Input/InputRouter.h"
#include "Window/Window.h"

#include "ImguiOgre/ImguiManager.h"

#include <cstring>

namespace AVImgui{

    namespace {
        AV::InputLayerHandle sLayerHandle = AV::INVALID_INPUT_LAYER;

        //The modifier mask the engine forwards is SDL's, whose values are fixed
        //by its ABI. Spelled out here rather than including SDL_keycode.h, to
        //keep this file's SDL surface to the scancode table alone.
        const int KEYMOD_SHIFT = 0x0003;
        const int KEYMOD_CTRL  = 0x00C0;
        const int KEYMOD_ALT   = 0x0300;
        const int KEYMOD_GUI   = 0x0C00;

        //imgui's display space is the render target in device pixels, while the
        //router reports the pointer in window (logical) points.
        inline void displayScale(float* outX, float* outY){
            *outX = 1.0f;
            *outY = 1.0f;

            ImGuiIO& io = ImGui::GetIO();
            AV::Window* window = AV::BaseSingleton::getWindow();
            if(!window || window->getWidth() <= 0 || window->getHeight() <= 0) return;

            *outX = io.DisplaySize.x / (float)window->getWidth();
            *outY = io.DisplaySize.y / (float)window->getHeight();
        }

        inline bool contextReady(){
            return ImGui::GetCurrentContext() != 0;
        }

        //Whether imgui currently wants the pointer. The NoMouse config flag is
        //honoured here so setMouseInputEnabled(false) also stops imgui
        //swallowing anything, rather than only stopping it reacting.
        inline bool imguiWantsMouse(){
            if(!contextReady()) return false;
            ImGuiIO& io = ImGui::GetIO();
            if(io.ConfigFlags & ImGuiConfigFlags_NoMouse) return false;
            return io.WantCaptureMouse;
        }
    }

    void ImguiInput::initialise(){
        std::shared_ptr<AV::InputRouter> router = AV::BaseSingleton::getInputRouter();
        if(!router) return;

        AV::InputLayerCallbacks cb;
        memset(&cb, 0, sizeof(AV::InputLayerCallbacks));
        cb.isLive = &ImguiInput::isLive;
        cb.wantsPointer = &ImguiInput::wantsPointer;
        cb.wantsKeyboard = &ImguiInput::wantsKeyboard;
        cb.wantsTextInput = &ImguiInput::wantsTextInput;
        cb.onMouseMove = &ImguiInput::onMouseMove;
        cb.onMouseButton = &ImguiInput::onMouseButton;
        cb.onMouseWheel = &ImguiInput::onMouseWheel;
        cb.onKey = &ImguiInput::onKey;
        cb.onTextInput = &ImguiInput::onTextInput;
        cb.onInputCancelled = &ImguiInput::onInputCancelled;

        sLayerHandle = router->addLayer("imgui", AV::INPUT_PRIORITY_OVERLAY, cb, 0);
    }

    void ImguiInput::shutdown(){
        std::shared_ptr<AV::InputRouter> router = AV::BaseSingleton::getInputRouter();
        if(router && sLayerHandle != AV::INVALID_INPUT_LAYER){
            router->removeLayer(sLayerHandle);
        }
        sLayerHandle = AV::INVALID_INPUT_LAYER;
    }

    bool ImguiInput::isLive(void*){
        if(!contextReady()) return false;

        //Between the plugin loading and the project's first imgui call there is
        //a context but nothing drawing, and the same is true again if a project
        //stops using imgui. Either way the capture flags describe nothing, so
        //the layer stays transparent.
        ImguiManager* manager = ImguiManager::getSingletonPtr();
        return manager && manager->isFrameLive();
    }

    bool ImguiInput::wantsPointer(void*){
        return imguiWantsMouse();
    }

    bool ImguiInput::wantsKeyboard(void*){
        if(!contextReady()) return false;
        return ImGui::GetIO().WantCaptureKeyboard;
    }

    bool ImguiInput::wantsTextInput(void*){
        if(!contextReady()) return false;
        //Asking for this is what makes the engine enable the OS text input
        //events, which is how imgui text fields get correctly laid out
        //characters rather than a guess reconstructed from scancodes.
        return ImGui::GetIO().WantTextInput;
    }

    void ImguiInput::onMouseMove(float x, float y, float, float, bool, void*){
        if(!contextReady()) return;

        //Fed regardless of whether imgui owns the pointer. imgui works out what
        //is hovered during the next NewFrame, so withholding the position while
        //it isn't capturing would mean it never discovers the cursor is over one
        //of its windows, and it would never start capturing at all.
        float scaleX, scaleY;
        displayScale(&scaleX, &scaleY);
        ImGui::GetIO().AddMousePosEvent(x * scaleX, y * scaleY);
    }

    bool ImguiInput::onMouseButton(int button, bool pressed, void*){
        if(!contextReady()) return false;

        ImGui::GetIO().AddMouseButtonEvent(button, pressed);
        return imguiWantsMouse();
    }

    bool ImguiInput::onMouseWheel(float x, float y, void*){
        if(!contextReady()) return false;

        ImGui::GetIO().AddMouseWheelEvent(x, y);
        return imguiWantsMouse();
    }

    bool ImguiInput::onKey(int scancode, int, int keyMod, bool pressed, void*){
        if(!contextReady()) return false;

        ImGuiIO& io = ImGui::GetIO();

        //Modifier state travels with the event rather than being polled, so it
        //stays correct even for keys the engine never sees held.
        io.AddKeyEvent(ImGuiMod_Shift, (keyMod & KEYMOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Ctrl, (keyMod & KEYMOD_CTRL) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (keyMod & KEYMOD_ALT) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (keyMod & KEYMOD_GUI) != 0);

        ImGuiKey key = scancodeToImGuiKey(scancode);
        if(key != ImGuiKey_None){
            io.AddKeyEvent(key, pressed);
        }

        return io.WantCaptureKeyboard;
    }

    bool ImguiInput::onTextInput(const char* text, void*){
        if(!contextReady()) return false;

        ImGuiIO& io = ImGui::GetIO();
        if(!io.WantTextInput) return false;

        //Real text from the OS, so this follows the user's keyboard layout and
        //input method rather than assuming a US layout.
        io.AddInputCharactersUTF8(text);
        return true;
    }

    void ImguiInput::onInputCancelled(void*){
        if(!contextReady()) return;

        //Everything held has been revoked, so drop imgui's idea of what is down
        //rather than leaving a key or button stuck.
        ImGui::GetIO().ClearInputKeys();
        ImGui::GetIO().ClearInputMouse();
    }

    //Physical-key map, adapted from the scancode fallback in imgui_impl_sdl2.
    ImGuiKey ImguiInput::scancodeToImGuiKey(int scancode){
        switch(scancode){
            case SDL_SCANCODE_TAB: return ImGuiKey_Tab;
            case SDL_SCANCODE_LEFT: return ImGuiKey_LeftArrow;
            case SDL_SCANCODE_RIGHT: return ImGuiKey_RightArrow;
            case SDL_SCANCODE_UP: return ImGuiKey_UpArrow;
            case SDL_SCANCODE_DOWN: return ImGuiKey_DownArrow;
            case SDL_SCANCODE_PAGEUP: return ImGuiKey_PageUp;
            case SDL_SCANCODE_PAGEDOWN: return ImGuiKey_PageDown;
            case SDL_SCANCODE_HOME: return ImGuiKey_Home;
            case SDL_SCANCODE_END: return ImGuiKey_End;
            case SDL_SCANCODE_INSERT: return ImGuiKey_Insert;
            case SDL_SCANCODE_DELETE: return ImGuiKey_Delete;
            case SDL_SCANCODE_BACKSPACE: return ImGuiKey_Backspace;
            case SDL_SCANCODE_SPACE: return ImGuiKey_Space;
            case SDL_SCANCODE_RETURN: return ImGuiKey_Enter;
            case SDL_SCANCODE_ESCAPE: return ImGuiKey_Escape;
            case SDL_SCANCODE_APOSTROPHE: return ImGuiKey_Apostrophe;
            case SDL_SCANCODE_COMMA: return ImGuiKey_Comma;
            case SDL_SCANCODE_MINUS: return ImGuiKey_Minus;
            case SDL_SCANCODE_PERIOD: return ImGuiKey_Period;
            case SDL_SCANCODE_SLASH: return ImGuiKey_Slash;
            case SDL_SCANCODE_SEMICOLON: return ImGuiKey_Semicolon;
            case SDL_SCANCODE_EQUALS: return ImGuiKey_Equal;
            case SDL_SCANCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
            case SDL_SCANCODE_BACKSLASH: return ImGuiKey_Backslash;
            case SDL_SCANCODE_RIGHTBRACKET: return ImGuiKey_RightBracket;
            case SDL_SCANCODE_GRAVE: return ImGuiKey_GraveAccent;
            case SDL_SCANCODE_CAPSLOCK: return ImGuiKey_CapsLock;
            case SDL_SCANCODE_SCROLLLOCK: return ImGuiKey_ScrollLock;
            case SDL_SCANCODE_NUMLOCKCLEAR: return ImGuiKey_NumLock;
            case SDL_SCANCODE_PRINTSCREEN: return ImGuiKey_PrintScreen;
            case SDL_SCANCODE_PAUSE: return ImGuiKey_Pause;
            case SDL_SCANCODE_KP_0: return ImGuiKey_Keypad0;
            case SDL_SCANCODE_KP_1: return ImGuiKey_Keypad1;
            case SDL_SCANCODE_KP_2: return ImGuiKey_Keypad2;
            case SDL_SCANCODE_KP_3: return ImGuiKey_Keypad3;
            case SDL_SCANCODE_KP_4: return ImGuiKey_Keypad4;
            case SDL_SCANCODE_KP_5: return ImGuiKey_Keypad5;
            case SDL_SCANCODE_KP_6: return ImGuiKey_Keypad6;
            case SDL_SCANCODE_KP_7: return ImGuiKey_Keypad7;
            case SDL_SCANCODE_KP_8: return ImGuiKey_Keypad8;
            case SDL_SCANCODE_KP_9: return ImGuiKey_Keypad9;
            case SDL_SCANCODE_KP_PERIOD: return ImGuiKey_KeypadDecimal;
            case SDL_SCANCODE_KP_DIVIDE: return ImGuiKey_KeypadDivide;
            case SDL_SCANCODE_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
            case SDL_SCANCODE_KP_MINUS: return ImGuiKey_KeypadSubtract;
            case SDL_SCANCODE_KP_PLUS: return ImGuiKey_KeypadAdd;
            case SDL_SCANCODE_KP_ENTER: return ImGuiKey_KeypadEnter;
            case SDL_SCANCODE_KP_EQUALS: return ImGuiKey_KeypadEqual;
            case SDL_SCANCODE_LCTRL: return ImGuiKey_LeftCtrl;
            case SDL_SCANCODE_LSHIFT: return ImGuiKey_LeftShift;
            case SDL_SCANCODE_LALT: return ImGuiKey_LeftAlt;
            case SDL_SCANCODE_LGUI: return ImGuiKey_LeftSuper;
            case SDL_SCANCODE_RCTRL: return ImGuiKey_RightCtrl;
            case SDL_SCANCODE_RSHIFT: return ImGuiKey_RightShift;
            case SDL_SCANCODE_RALT: return ImGuiKey_RightAlt;
            case SDL_SCANCODE_RGUI: return ImGuiKey_RightSuper;
            case SDL_SCANCODE_APPLICATION: return ImGuiKey_Menu;
            case SDL_SCANCODE_0: return ImGuiKey_0;
            case SDL_SCANCODE_1: return ImGuiKey_1;
            case SDL_SCANCODE_2: return ImGuiKey_2;
            case SDL_SCANCODE_3: return ImGuiKey_3;
            case SDL_SCANCODE_4: return ImGuiKey_4;
            case SDL_SCANCODE_5: return ImGuiKey_5;
            case SDL_SCANCODE_6: return ImGuiKey_6;
            case SDL_SCANCODE_7: return ImGuiKey_7;
            case SDL_SCANCODE_8: return ImGuiKey_8;
            case SDL_SCANCODE_9: return ImGuiKey_9;
            case SDL_SCANCODE_A: return ImGuiKey_A;
            case SDL_SCANCODE_B: return ImGuiKey_B;
            case SDL_SCANCODE_C: return ImGuiKey_C;
            case SDL_SCANCODE_D: return ImGuiKey_D;
            case SDL_SCANCODE_E: return ImGuiKey_E;
            case SDL_SCANCODE_F: return ImGuiKey_F;
            case SDL_SCANCODE_G: return ImGuiKey_G;
            case SDL_SCANCODE_H: return ImGuiKey_H;
            case SDL_SCANCODE_I: return ImGuiKey_I;
            case SDL_SCANCODE_J: return ImGuiKey_J;
            case SDL_SCANCODE_K: return ImGuiKey_K;
            case SDL_SCANCODE_L: return ImGuiKey_L;
            case SDL_SCANCODE_M: return ImGuiKey_M;
            case SDL_SCANCODE_N: return ImGuiKey_N;
            case SDL_SCANCODE_O: return ImGuiKey_O;
            case SDL_SCANCODE_P: return ImGuiKey_P;
            case SDL_SCANCODE_Q: return ImGuiKey_Q;
            case SDL_SCANCODE_R: return ImGuiKey_R;
            case SDL_SCANCODE_S: return ImGuiKey_S;
            case SDL_SCANCODE_T: return ImGuiKey_T;
            case SDL_SCANCODE_U: return ImGuiKey_U;
            case SDL_SCANCODE_V: return ImGuiKey_V;
            case SDL_SCANCODE_W: return ImGuiKey_W;
            case SDL_SCANCODE_X: return ImGuiKey_X;
            case SDL_SCANCODE_Y: return ImGuiKey_Y;
            case SDL_SCANCODE_Z: return ImGuiKey_Z;
            case SDL_SCANCODE_F1: return ImGuiKey_F1;
            case SDL_SCANCODE_F2: return ImGuiKey_F2;
            case SDL_SCANCODE_F3: return ImGuiKey_F3;
            case SDL_SCANCODE_F4: return ImGuiKey_F4;
            case SDL_SCANCODE_F5: return ImGuiKey_F5;
            case SDL_SCANCODE_F6: return ImGuiKey_F6;
            case SDL_SCANCODE_F7: return ImGuiKey_F7;
            case SDL_SCANCODE_F8: return ImGuiKey_F8;
            case SDL_SCANCODE_F9: return ImGuiKey_F9;
            case SDL_SCANCODE_F10: return ImGuiKey_F10;
            case SDL_SCANCODE_F11: return ImGuiKey_F11;
            case SDL_SCANCODE_F12: return ImGuiKey_F12;
            default: return ImGuiKey_None;
        }
    }
}
