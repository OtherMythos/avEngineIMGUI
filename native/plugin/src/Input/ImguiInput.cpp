#include "ImguiInput.h"

#include <cfloat>

namespace AVImgui{

    void ImguiInput::initialise(){
        SDL_AddEventWatch(&ImguiInput::eventWatch, 0);
    }

    void ImguiInput::shutdown(){
        SDL_DelEventWatch(&ImguiInput::eventWatch, 0);
    }

    void ImguiInput::applyTextInputState(){
        ImGuiIO& io = ImGui::GetIO();
        if(io.WantTextInput && !SDL_IsTextInputActive()){
            SDL_StartTextInput();
        }
        //Disabling text input is deliberately left to the engine, which toggles
        //it based on its own gui focus. Leaving it enabled is harmless.
    }

    void ImguiInput::updateKeyModifiers(SDL_Keymod sdlKeyMods){
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiMod_Ctrl, (sdlKeyMods & KMOD_CTRL) != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (sdlKeyMods & KMOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Alt, (sdlKeyMods & KMOD_ALT) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (sdlKeyMods & KMOD_GUI) != 0);
    }

    int SDLCALL ImguiInput::eventWatch(void* userdata, SDL_Event* event){
        if(ImGui::GetCurrentContext() == 0) return 0;
        ImGuiIO& io = ImGui::GetIO();

        switch(event->type){
            case SDL_MOUSEMOTION:{
                io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMousePosEvent((float)event->motion.x, (float)event->motion.y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:{
                int mouseButton = -1;
                if(event->button.button == SDL_BUTTON_LEFT) mouseButton = 0;
                if(event->button.button == SDL_BUTTON_RIGHT) mouseButton = 1;
                if(event->button.button == SDL_BUTTON_MIDDLE) mouseButton = 2;
                if(event->button.button == SDL_BUTTON_X1) mouseButton = 3;
                if(event->button.button == SDL_BUTTON_X2) mouseButton = 4;
                if(mouseButton == -1) break;
                io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMouseButtonEvent(mouseButton, event->type == SDL_MOUSEBUTTONDOWN);
                break;
            }
            case SDL_MOUSEWHEEL:{
                #if SDL_VERSION_ATLEAST(2,0,18)
                    float wheelX = -event->wheel.preciseX;
                    float wheelY = event->wheel.preciseY;
                #else
                    float wheelX = -(float)event->wheel.x;
                    float wheelY = (float)event->wheel.y;
                #endif
                io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
                io.AddMouseWheelEvent(wheelX, wheelY);
                break;
            }
            case SDL_TEXTINPUT:{
                io.AddInputCharactersUTF8(event->text.text);
                break;
            }
            case SDL_KEYDOWN:
            case SDL_KEYUP:{
                updateKeyModifiers((SDL_Keymod)event->key.keysym.mod);
                ImGuiKey key = keyEventToImGuiKey(event->key.keysym.sym, event->key.keysym.scancode);
                if(key != ImGuiKey_None){
                    io.AddKeyEvent(key, event->type == SDL_KEYDOWN);
                }
                break;
            }
            case SDL_WINDOWEVENT:{
                Uint8 windowEvent = event->window.event;
                if(windowEvent == SDL_WINDOWEVENT_LEAVE){
                    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
                }
                if(windowEvent == SDL_WINDOWEVENT_FOCUS_GAINED){
                    io.AddFocusEvent(true);
                }
                if(windowEvent == SDL_WINDOWEVENT_FOCUS_LOST){
                    io.AddFocusEvent(false);
                }
                break;
            }
            default:
                break;
        }

        return 0;
    }

    //Adapted from the official imgui_impl_sdl2 backend.
    ImGuiKey ImguiInput::keyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode){
        switch(keycode){
            case SDLK_TAB: return ImGuiKey_Tab;
            case SDLK_LEFT: return ImGuiKey_LeftArrow;
            case SDLK_RIGHT: return ImGuiKey_RightArrow;
            case SDLK_UP: return ImGuiKey_UpArrow;
            case SDLK_DOWN: return ImGuiKey_DownArrow;
            case SDLK_PAGEUP: return ImGuiKey_PageUp;
            case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
            case SDLK_HOME: return ImGuiKey_Home;
            case SDLK_END: return ImGuiKey_End;
            case SDLK_INSERT: return ImGuiKey_Insert;
            case SDLK_DELETE: return ImGuiKey_Delete;
            case SDLK_BACKSPACE: return ImGuiKey_Backspace;
            case SDLK_SPACE: return ImGuiKey_Space;
            case SDLK_RETURN: return ImGuiKey_Enter;
            case SDLK_ESCAPE: return ImGuiKey_Escape;
            case SDLK_COMMA: return ImGuiKey_Comma;
            case SDLK_PERIOD: return ImGuiKey_Period;
            case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
            case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
            case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
            case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
            case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
            case SDLK_PAUSE: return ImGuiKey_Pause;
            case SDLK_KP_0: return ImGuiKey_Keypad0;
            case SDLK_KP_1: return ImGuiKey_Keypad1;
            case SDLK_KP_2: return ImGuiKey_Keypad2;
            case SDLK_KP_3: return ImGuiKey_Keypad3;
            case SDLK_KP_4: return ImGuiKey_Keypad4;
            case SDLK_KP_5: return ImGuiKey_Keypad5;
            case SDLK_KP_6: return ImGuiKey_Keypad6;
            case SDLK_KP_7: return ImGuiKey_Keypad7;
            case SDLK_KP_8: return ImGuiKey_Keypad8;
            case SDLK_KP_9: return ImGuiKey_Keypad9;
            case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
            case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
            case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
            case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
            case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
            case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
            case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
            case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
            case SDLK_LSHIFT: return ImGuiKey_LeftShift;
            case SDLK_LALT: return ImGuiKey_LeftAlt;
            case SDLK_LGUI: return ImGuiKey_LeftSuper;
            case SDLK_RCTRL: return ImGuiKey_RightCtrl;
            case SDLK_RSHIFT: return ImGuiKey_RightShift;
            case SDLK_RALT: return ImGuiKey_RightAlt;
            case SDLK_RGUI: return ImGuiKey_RightSuper;
            case SDLK_APPLICATION: return ImGuiKey_Menu;
            case SDLK_0: return ImGuiKey_0;
            case SDLK_1: return ImGuiKey_1;
            case SDLK_2: return ImGuiKey_2;
            case SDLK_3: return ImGuiKey_3;
            case SDLK_4: return ImGuiKey_4;
            case SDLK_5: return ImGuiKey_5;
            case SDLK_6: return ImGuiKey_6;
            case SDLK_7: return ImGuiKey_7;
            case SDLK_8: return ImGuiKey_8;
            case SDLK_9: return ImGuiKey_9;
            case SDLK_a: return ImGuiKey_A;
            case SDLK_b: return ImGuiKey_B;
            case SDLK_c: return ImGuiKey_C;
            case SDLK_d: return ImGuiKey_D;
            case SDLK_e: return ImGuiKey_E;
            case SDLK_f: return ImGuiKey_F;
            case SDLK_g: return ImGuiKey_G;
            case SDLK_h: return ImGuiKey_H;
            case SDLK_i: return ImGuiKey_I;
            case SDLK_j: return ImGuiKey_J;
            case SDLK_k: return ImGuiKey_K;
            case SDLK_l: return ImGuiKey_L;
            case SDLK_m: return ImGuiKey_M;
            case SDLK_n: return ImGuiKey_N;
            case SDLK_o: return ImGuiKey_O;
            case SDLK_p: return ImGuiKey_P;
            case SDLK_q: return ImGuiKey_Q;
            case SDLK_r: return ImGuiKey_R;
            case SDLK_s: return ImGuiKey_S;
            case SDLK_t: return ImGuiKey_T;
            case SDLK_u: return ImGuiKey_U;
            case SDLK_v: return ImGuiKey_V;
            case SDLK_w: return ImGuiKey_W;
            case SDLK_x: return ImGuiKey_X;
            case SDLK_y: return ImGuiKey_Y;
            case SDLK_z: return ImGuiKey_Z;
            case SDLK_F1: return ImGuiKey_F1;
            case SDLK_F2: return ImGuiKey_F2;
            case SDLK_F3: return ImGuiKey_F3;
            case SDLK_F4: return ImGuiKey_F4;
            case SDLK_F5: return ImGuiKey_F5;
            case SDLK_F6: return ImGuiKey_F6;
            case SDLK_F7: return ImGuiKey_F7;
            case SDLK_F8: return ImGuiKey_F8;
            case SDLK_F9: return ImGuiKey_F9;
            case SDLK_F10: return ImGuiKey_F10;
            case SDLK_F11: return ImGuiKey_F11;
            case SDLK_F12: return ImGuiKey_F12;
            case SDLK_F13: return ImGuiKey_F13;
            case SDLK_F14: return ImGuiKey_F14;
            case SDLK_F15: return ImGuiKey_F15;
            case SDLK_F16: return ImGuiKey_F16;
            case SDLK_F17: return ImGuiKey_F17;
            case SDLK_F18: return ImGuiKey_F18;
            case SDLK_F19: return ImGuiKey_F19;
            case SDLK_F20: return ImGuiKey_F20;
            case SDLK_F21: return ImGuiKey_F21;
            case SDLK_F22: return ImGuiKey_F22;
            case SDLK_F23: return ImGuiKey_F23;
            case SDLK_F24: return ImGuiKey_F24;
            case SDLK_AC_BACK: return ImGuiKey_AppBack;
            case SDLK_AC_FORWARD: return ImGuiKey_AppForward;
            default: break;
        }

        //Fallback to scancode for keys whose keycode depends on the layout.
        switch(scancode){
            case SDL_SCANCODE_GRAVE: return ImGuiKey_GraveAccent;
            case SDL_SCANCODE_MINUS: return ImGuiKey_Minus;
            case SDL_SCANCODE_EQUALS: return ImGuiKey_Equal;
            case SDL_SCANCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
            case SDL_SCANCODE_RIGHTBRACKET: return ImGuiKey_RightBracket;
            case SDL_SCANCODE_NONUSBACKSLASH: return ImGuiKey_Oem102;
            case SDL_SCANCODE_BACKSLASH: return ImGuiKey_Backslash;
            case SDL_SCANCODE_SEMICOLON: return ImGuiKey_Semicolon;
            case SDL_SCANCODE_APOSTROPHE: return ImGuiKey_Apostrophe;
            case SDL_SCANCODE_COMMA: return ImGuiKey_Comma;
            case SDL_SCANCODE_PERIOD: return ImGuiKey_Period;
            case SDL_SCANCODE_SLASH: return ImGuiKey_Slash;
            default: break;
        }
        return ImGuiKey_None;
    }
}
