#include "ImguiInput.h"

//Only the scancode enum is pulled in - a compile-time constant table, no SDL
//runtime symbols (the engine statically links SDL and does not re-export it).
#include <SDL_scancode.h>

#include "System/BaseSingleton.h"
#include "Input/InputManager.h"
#include "Window/Window.h"

namespace AVImgui{

    namespace {
        const int SCANCODE_COUNT = SDL_NUM_SCANCODES;
        //Previous frame's key state, for edge detection.
        static bool sLastKeys[SDL_NUM_SCANCODES] = { false };
        static bool sHasLast = false;

        inline bool keyDown(AV::InputManager* input, int scancode){
            return input->getKeyboardInput(scancode);
        }

        //US-layout ASCII for a printable scancode, or 0. shifted picks the
        //upper symbol. This is a best-effort for text fields; it does not follow
        //other keyboard layouts.
        char printableChar(int scancode, bool shift){
            if(scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z){
                char base = 'a' + (scancode - SDL_SCANCODE_A);
                return shift ? (char)(base - 32) : base;
            }
            if(scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9){
                static const char* shifted = "!@#$%^&*(";
                return shift ? shifted[scancode - SDL_SCANCODE_1] : (char)('1' + (scancode - SDL_SCANCODE_1));
            }
            switch(scancode){
                case SDL_SCANCODE_0: return shift ? ')' : '0';
                case SDL_SCANCODE_SPACE: return ' ';
                case SDL_SCANCODE_MINUS: return shift ? '_' : '-';
                case SDL_SCANCODE_EQUALS: return shift ? '+' : '=';
                case SDL_SCANCODE_LEFTBRACKET: return shift ? '{' : '[';
                case SDL_SCANCODE_RIGHTBRACKET: return shift ? '}' : ']';
                case SDL_SCANCODE_BACKSLASH: return shift ? '|' : '\\';
                case SDL_SCANCODE_SEMICOLON: return shift ? ':' : ';';
                case SDL_SCANCODE_APOSTROPHE: return shift ? '"' : '\'';
                case SDL_SCANCODE_GRAVE: return shift ? '~' : '`';
                case SDL_SCANCODE_COMMA: return shift ? '<' : ',';
                case SDL_SCANCODE_PERIOD: return shift ? '>' : '.';
                case SDL_SCANCODE_SLASH: return shift ? '?' : '/';
                default: return 0;
            }
        }
    }

    void ImguiInput::initialise(){
        sHasLast = false;
        for(int i = 0; i < SCANCODE_COUNT; i++) sLastKeys[i] = false;
    }

    void ImguiInput::shutdown(){
    }

    void ImguiInput::update(){
        if(ImGui::GetCurrentContext() == 0) return;
        AV::InputManager* input = AV::BaseSingleton::getInputManager().get();
        if(!input) return;

        ImGuiIO& io = ImGui::GetIO();

        //Mouse. imgui's display space is the render target in device pixels,
        //while InputManager reports the pointer in window (logical) points, so
        //scale by display/window. This deliberately does not use
        //getActualMouseX(): that is only populated by real SDL motion, whereas
        //the logical position is also what the debug server injects, so this
        //path works when input is driven programmatically too.
        AV::Window* window = AV::BaseSingleton::getWindow();
        float scaleX = 1.0f, scaleY = 1.0f;
        if(window && window->getWidth() > 0 && window->getHeight() > 0){
            scaleX = io.DisplaySize.x / (float)window->getWidth();
            scaleY = io.DisplaySize.y / (float)window->getHeight();
        }
        io.AddMousePosEvent((float)input->getMouseX() * scaleX, (float)input->getMouseY() * scaleY);
        io.AddMouseButtonEvent(0, input->getMouseButton(0));
        io.AddMouseButtonEvent(1, input->getMouseButton(1));
        io.AddMouseButtonEvent(2, input->getMouseButton(2));

        int wheel = input->getMouseWheel();
        if(wheel != 0){
            io.AddMouseWheelEvent(0.0f, (float)wheel);
        }

        //Modifiers.
        bool shift = keyDown(input, SDL_SCANCODE_LSHIFT) || keyDown(input, SDL_SCANCODE_RSHIFT);
        bool ctrl  = keyDown(input, SDL_SCANCODE_LCTRL)  || keyDown(input, SDL_SCANCODE_RCTRL);
        bool alt   = keyDown(input, SDL_SCANCODE_LALT)   || keyDown(input, SDL_SCANCODE_RALT);
        bool super = keyDown(input, SDL_SCANCODE_LGUI)   || keyDown(input, SDL_SCANCODE_RGUI);
        io.AddKeyEvent(ImGuiMod_Shift, shift);
        io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);
        io.AddKeyEvent(ImGuiMod_Alt, alt);
        io.AddKeyEvent(ImGuiMod_Super, super);

        //Keys: diff against the previous frame so each change becomes an event.
        for(int sc = 0; sc < SCANCODE_COUNT; sc++){
            bool down = input->getKeyboardInput(sc);
            bool was = sHasLast ? sLastKeys[sc] : false;
            if(down == was) continue;

            ImGuiKey key = scancodeToImGuiKey(sc);
            if(key != ImGuiKey_None){
                io.AddKeyEvent(key, down);
            }

            //On a fresh press of a printable key, feed the character too, so
            //text fields accept typing (US layout, ASCII only).
            if(down && io.WantTextInput){
                char c = printableChar(sc, shift);
                if(c != 0) io.AddInputCharacter((unsigned int)c);
            }

            sLastKeys[sc] = down;
        }
        sHasLast = true;
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
