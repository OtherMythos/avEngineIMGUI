#include "ImguiNamespace.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "imgui.h"

#include "AvImguiPlugin.h"
#include "ImguiOgre/ImguiManager.h"

#include <cfloat>
#include <string>
#include <vector>

namespace AVImgui{

    namespace {

        //Every widget binding begins the imgui frame if this engine frame has
        //not begun one yet, so scripts never manage the frame lifecycle.
        inline bool frameGuard(){
            ImguiManager* manager = ImguiManager::getSingletonPtr();
            if(!manager) return false;
            manager->ensureFrameStarted();
            return true;
        }

        inline SQFloat getFloatOr(HSQUIRRELVM vm, SQInteger idx, SQFloat def){
            if(sq_gettop(vm) >= idx){
                SQFloat v;
                if(SQ_SUCCEEDED(sq_getfloat(vm, idx, &v))) return v;
            }
            return def;
        }

        inline SQInteger getIntOr(HSQUIRRELVM vm, SQInteger idx, SQInteger def){
            if(sq_gettop(vm) >= idx){
                SQInteger v;
                if(SQ_SUCCEEDED(sq_getinteger(vm, idx, &v))) return v;
            }
            return def;
        }

        inline bool getBoolOr(HSQUIRRELVM vm, SQInteger idx, bool def){
            if(sq_gettop(vm) >= idx){
                SQBool v;
                if(SQ_SUCCEEDED(sq_getbool(vm, idx, &v))) return v != SQFalse;
            }
            return def;
        }

        inline const SQChar* getStringOr(HSQUIRRELVM vm, SQInteger idx, const SQChar* def){
            if(sq_gettop(vm) >= idx && sq_gettype(vm, idx) == OT_STRING){
                const SQChar* v;
                sq_getstring(vm, idx, &v);
                return v;
            }
            return def;
        }

        inline SQInteger pushVec2(HSQUIRRELVM vm, float x, float y){
            sq_newarray(vm, 0);
            sq_pushfloat(vm, x);
            sq_arrayappend(vm, -2);
            sq_pushfloat(vm, y);
            sq_arrayappend(vm, -2);
            return 1;
        }

        //Read up to maxCount floats from the array at idx. Returns the count read.
        inline int readFloatArray(HSQUIRRELVM vm, SQInteger idx, float* out, int maxCount){
            SQInteger size = sq_getsize(vm, idx);
            int count = (int)(size < maxCount ? size : maxCount);
            for(int i = 0; i < count; i++){
                sq_pushinteger(vm, i);
                if(SQ_FAILED(sq_get(vm, idx))) return i;
                SQFloat v = 0;
                sq_getfloat(vm, -1, &v);
                sq_pop(vm, 1);
                out[i] = (float)v;
            }
            return count;
        }

        //Write floats back into the array at idx.
        inline void writeFloatArray(HSQUIRRELVM vm, SQInteger idx, const float* vals, int count){
            for(int i = 0; i < count; i++){
                sq_pushinteger(vm, i);
                sq_pushfloat(vm, vals[i]);
                sq_set(vm, idx);
            }
        }

        //Read an array of strings. Storage keeps the std::strings alive while
        //the const char* pointers are in use during the imgui call.
        inline void readStringArray(HSQUIRRELVM vm, SQInteger idx, std::vector<std::string>& storage, std::vector<const char*>& pointers){
            SQInteger size = sq_getsize(vm, idx);
            storage.reserve(size);
            pointers.reserve(size);
            for(SQInteger i = 0; i < size; i++){
                sq_pushinteger(vm, i);
                if(SQ_FAILED(sq_get(vm, idx))) break;
                const SQChar* str = "";
                sq_getstring(vm, -1, &str);
                storage.push_back(str);
                sq_pop(vm, 1);
            }
            for(const std::string& s : storage){
                pointers.push_back(s.c_str());
            }
        }

        #define IMGUI_FRAME_GUARD if(!frameGuard()) return 0;

        //---------------------------------------------------------------------
        //Demo, metrics, misc
        //---------------------------------------------------------------------
        SQInteger showDemoWindow(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::ShowDemoWindow();
            return 0;
        }
        SQInteger showMetricsWindow(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::ShowMetricsWindow();
            return 0;
        }
        SQInteger getVersion(HSQUIRRELVM vm){
            sq_pushstring(vm, ImGui::GetVersion(), -1);
            return 1;
        }
        SQInteger getTime(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, (SQFloat)ImGui::GetTime());
            return 1;
        }
        SQInteger getFrameCount(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushinteger(vm, ImGui::GetFrameCount());
            return 1;
        }
        SQInteger getFramerate(HSQUIRRELVM vm){
            //Smoothed over recent frames by imgui. Measured per rendered frame,
            //so this is the real framerate rather than the engine's fixed
            //script update rate.
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetIO().Framerate);
            return 1;
        }
        SQInteger getDeltaTime(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetIO().DeltaTime);
            return 1;
        }
        SQInteger getDisplaySize(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const ImVec2 size = ImGui::GetIO().DisplaySize;
            return pushVec2(vm, size.x, size.y);
        }
        SQInteger wantCaptureMouse(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::GetIO().WantCaptureMouse);
            return 1;
        }
        SQInteger wantCaptureKeyboard(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::GetIO().WantCaptureKeyboard);
            return 1;
        }
        SQInteger wantTextInput(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::GetIO().WantTextInput);
            return 1;
        }
        SQInteger isFirstUpdateOfFrame(HSQUIRRELVM vm){
            //Deliberately does not begin the frame. The engine runs script
            //updates on a fixed timestep, so a single rendered frame can
            //receive more than one update call. Scripts which want to avoid
            //submitting their gui twice in that case can early-out:
            //  if(!_imgui.isFirstUpdateOfFrame()) return;
            ImguiManager* manager = ImguiManager::getSingletonPtr();
            sq_pushbool(vm, manager ? manager->wouldBeFirstUpdateOfFrame() : true);
            return 1;
        }
        SQInteger setRenderingEnabled(HSQUIRRELVM vm){
            SQBool enabled;
            sq_getbool(vm, 2, &enabled);
            ImguiManager* manager = ImguiManager::getSingletonPtr();
            if(manager) manager->setRenderingEnabled(enabled != SQFalse);
            return 0;
        }
        SQInteger createOverlayWorkspace(HSQUIRRELVM vm){
            sq_pushbool(vm, AvImguiPlugin::createOverlayWorkspace());
            return 1;
        }
        SQInteger destroyOverlayWorkspace(HSQUIRRELVM vm){
            sq_pushbool(vm, AvImguiPlugin::destroyOverlayWorkspace());
            return 1;
        }
        SQInteger setAutoOverlayEnabled(HSQUIRRELVM vm){
            SQBool enabled;
            sq_getbool(vm, 2, &enabled);
            AvImguiPlugin::setAutoOverlayEnabled(enabled != SQFalse);
            return 0;
        }
        SQInteger getAutoOverlayEnabled(HSQUIRRELVM vm){
            sq_pushbool(vm, AvImguiPlugin::getAutoOverlayEnabled());
            return 1;
        }

        //---------------------------------------------------------------------
        //Windows
        //---------------------------------------------------------------------
        SQInteger begin(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* name;
            sq_getstring(vm, 2, &name);
            ImGuiWindowFlags flags = (ImGuiWindowFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::Begin(name, 0, flags));
            return 1;
        }
        SQInteger end(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::End();
            return 0;
        }
        SQInteger beginChild(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            float w = (float)getFloatOr(vm, 3, 0.0f);
            float h = (float)getFloatOr(vm, 4, 0.0f);
            ImGuiChildFlags childFlags = (ImGuiChildFlags)getIntOr(vm, 5, 0);
            ImGuiWindowFlags windowFlags = (ImGuiWindowFlags)getIntOr(vm, 6, 0);
            sq_pushbool(vm, ImGui::BeginChild(id, ImVec2(w, h), childFlags, windowFlags));
            return 1;
        }
        SQInteger endChild(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndChild();
            return 0;
        }
        SQInteger setNextWindowPos(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat x, y;
            sq_getfloat(vm, 2, &x);
            sq_getfloat(vm, 3, &y);
            ImGuiCond cond = (ImGuiCond)getIntOr(vm, 4, 0);
            float pivotX = (float)getFloatOr(vm, 5, 0.0f);
            float pivotY = (float)getFloatOr(vm, 6, 0.0f);
            ImGui::SetNextWindowPos(ImVec2((float)x, (float)y), cond, ImVec2(pivotX, pivotY));
            return 0;
        }
        SQInteger setNextWindowSize(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat w, h;
            sq_getfloat(vm, 2, &w);
            sq_getfloat(vm, 3, &h);
            ImGuiCond cond = (ImGuiCond)getIntOr(vm, 4, 0);
            ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), cond);
            return 0;
        }
        SQInteger setNextWindowCollapsed(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQBool collapsed;
            sq_getbool(vm, 2, &collapsed);
            ImGuiCond cond = (ImGuiCond)getIntOr(vm, 3, 0);
            ImGui::SetNextWindowCollapsed(collapsed != SQFalse, cond);
            return 0;
        }
        SQInteger setNextWindowFocus(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::SetNextWindowFocus();
            return 0;
        }
        SQInteger setNextWindowBgAlpha(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat a;
            sq_getfloat(vm, 2, &a);
            ImGui::SetNextWindowBgAlpha((float)a);
            return 0;
        }
        SQInteger getWindowPos(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const ImVec2 v = ImGui::GetWindowPos();
            return pushVec2(vm, v.x, v.y);
        }
        SQInteger getWindowSize(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const ImVec2 v = ImGui::GetWindowSize();
            return pushVec2(vm, v.x, v.y);
        }
        SQInteger getWindowWidth(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetWindowWidth());
            return 1;
        }
        SQInteger getWindowHeight(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetWindowHeight());
            return 1;
        }
        SQInteger getContentRegionAvail(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const ImVec2 v = ImGui::GetContentRegionAvail();
            return pushVec2(vm, v.x, v.y);
        }
        SQInteger isWindowHovered(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsWindowHovered((ImGuiHoveredFlags)getIntOr(vm, 2, 0)));
            return 1;
        }
        SQInteger isWindowFocused(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsWindowFocused((ImGuiFocusedFlags)getIntOr(vm, 2, 0)));
            return 1;
        }
        SQInteger isWindowCollapsed(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsWindowCollapsed());
            return 1;
        }

        //---------------------------------------------------------------------
        //Text
        //---------------------------------------------------------------------
        SQInteger text(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::TextUnformatted(str);
            return 0;
        }
        SQInteger textColored(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat r, g, b, a;
            sq_getfloat(vm, 2, &r);
            sq_getfloat(vm, 3, &g);
            sq_getfloat(vm, 4, &b);
            sq_getfloat(vm, 5, &a);
            const SQChar* str;
            sq_getstring(vm, 6, &str);
            ImGui::TextColored(ImVec4((float)r, (float)g, (float)b, (float)a), "%s", str);
            return 0;
        }
        SQInteger textDisabled(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::TextDisabled("%s", str);
            return 0;
        }
        SQInteger textWrapped(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::TextWrapped("%s", str);
            return 0;
        }
        SQInteger bulletText(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::BulletText("%s", str);
            return 0;
        }
        SQInteger labelText(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            const SQChar* str;
            sq_getstring(vm, 2, &label);
            sq_getstring(vm, 3, &str);
            ImGui::LabelText(label, "%s", str);
            return 0;
        }
        SQInteger separatorText(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::SeparatorText(str);
            return 0;
        }

        //---------------------------------------------------------------------
        //Main widgets
        //---------------------------------------------------------------------
        SQInteger button(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            float w = (float)getFloatOr(vm, 3, 0.0f);
            float h = (float)getFloatOr(vm, 4, 0.0f);
            sq_pushbool(vm, ImGui::Button(label, ImVec2(w, h)));
            return 1;
        }
        SQInteger smallButton(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            sq_pushbool(vm, ImGui::SmallButton(label));
            return 1;
        }
        SQInteger invisibleButton(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            SQFloat w, h;
            sq_getfloat(vm, 3, &w);
            sq_getfloat(vm, 4, &h);
            ImGuiButtonFlags flags = (ImGuiButtonFlags)getIntOr(vm, 5, 0);
            sq_pushbool(vm, ImGui::InvisibleButton(id, ImVec2((float)w, (float)h), flags));
            return 1;
        }
        SQInteger arrowButton(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            SQInteger dir;
            sq_getinteger(vm, 3, &dir);
            sq_pushbool(vm, ImGui::ArrowButton(id, (ImGuiDir)dir));
            return 1;
        }
        SQInteger checkbox(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQBool value;
            sq_getbool(vm, 3, &value);
            bool checked = value != SQFalse;
            ImGui::Checkbox(label, &checked);
            sq_pushbool(vm, checked);
            return 1;
        }
        SQInteger radioButton(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQBool active;
            sq_getbool(vm, 3, &active);
            sq_pushbool(vm, ImGui::RadioButton(label, active != SQFalse));
            return 1;
        }
        SQInteger selectable(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            bool selected = getBoolOr(vm, 3, false);
            ImGuiSelectableFlags flags = (ImGuiSelectableFlags)getIntOr(vm, 4, 0);
            float w = (float)getFloatOr(vm, 5, 0.0f);
            float h = (float)getFloatOr(vm, 6, 0.0f);
            sq_pushbool(vm, ImGui::Selectable(label, selected, flags, ImVec2(w, h)));
            return 1;
        }
        SQInteger progressBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat fraction;
            sq_getfloat(vm, 2, &fraction);
            float w = (float)getFloatOr(vm, 3, -FLT_MIN);
            float h = (float)getFloatOr(vm, 4, 0.0f);
            const SQChar* overlay = getStringOr(vm, 5, 0);
            ImGui::ProgressBar((float)fraction, ImVec2(w, h), overlay);
            return 0;
        }
        SQInteger bullet(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::Bullet();
            return 0;
        }

        //---------------------------------------------------------------------
        //Value widgets. These take the current value and return the new value.
        //---------------------------------------------------------------------
        SQInteger sliderFloat(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQFloat value, minVal, maxVal;
            sq_getfloat(vm, 3, &value);
            sq_getfloat(vm, 4, &minVal);
            sq_getfloat(vm, 5, &maxVal);
            const SQChar* format = getStringOr(vm, 6, "%.3f");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 7, 0);
            float v = (float)value;
            ImGui::SliderFloat(label, &v, (float)minVal, (float)maxVal, format, flags);
            sq_pushfloat(vm, v);
            return 1;
        }
        SQInteger sliderInt(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQInteger value, minVal, maxVal;
            sq_getinteger(vm, 3, &value);
            sq_getinteger(vm, 4, &minVal);
            sq_getinteger(vm, 5, &maxVal);
            const SQChar* format = getStringOr(vm, 6, "%d");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 7, 0);
            int v = (int)value;
            ImGui::SliderInt(label, &v, (int)minVal, (int)maxVal, format, flags);
            sq_pushinteger(vm, v);
            return 1;
        }
        SQInteger sliderFloatN(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            float values[4];
            int count = readFloatArray(vm, 3, values, 4);
            SQFloat minVal, maxVal;
            sq_getfloat(vm, 4, &minVal);
            sq_getfloat(vm, 5, &maxVal);
            const SQChar* format = getStringOr(vm, 6, "%.3f");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 7, 0);

            bool changed = false;
            switch(count){
                case 1: changed = ImGui::SliderFloat(label, values, (float)minVal, (float)maxVal, format, flags); break;
                case 2: changed = ImGui::SliderFloat2(label, values, (float)minVal, (float)maxVal, format, flags); break;
                case 3: changed = ImGui::SliderFloat3(label, values, (float)minVal, (float)maxVal, format, flags); break;
                case 4: changed = ImGui::SliderFloat4(label, values, (float)minVal, (float)maxVal, format, flags); break;
                default: return sq_throwerror(vm, "sliderFloatN expects an array of 1 to 4 numbers.");
            }
            if(changed) writeFloatArray(vm, 3, values, count);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger dragFloat(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQFloat value;
            sq_getfloat(vm, 3, &value);
            float speed = (float)getFloatOr(vm, 4, 1.0f);
            float minVal = (float)getFloatOr(vm, 5, 0.0f);
            float maxVal = (float)getFloatOr(vm, 6, 0.0f);
            const SQChar* format = getStringOr(vm, 7, "%.3f");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 8, 0);
            float v = (float)value;
            ImGui::DragFloat(label, &v, speed, minVal, maxVal, format, flags);
            sq_pushfloat(vm, v);
            return 1;
        }
        SQInteger dragInt(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQInteger value;
            sq_getinteger(vm, 3, &value);
            float speed = (float)getFloatOr(vm, 4, 1.0f);
            int minVal = (int)getIntOr(vm, 5, 0);
            int maxVal = (int)getIntOr(vm, 6, 0);
            const SQChar* format = getStringOr(vm, 7, "%d");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 8, 0);
            int v = (int)value;
            ImGui::DragInt(label, &v, speed, minVal, maxVal, format, flags);
            sq_pushinteger(vm, v);
            return 1;
        }
        SQInteger inputFloat(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQFloat value;
            sq_getfloat(vm, 3, &value);
            float step = (float)getFloatOr(vm, 4, 0.0f);
            float stepFast = (float)getFloatOr(vm, 5, 0.0f);
            const SQChar* format = getStringOr(vm, 6, "%.3f");
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 7, 0);
            float v = (float)value;
            ImGui::InputFloat(label, &v, step, stepFast, format, flags);
            sq_pushfloat(vm, v);
            return 1;
        }
        SQInteger inputInt(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQInteger value;
            sq_getinteger(vm, 3, &value);
            int step = (int)getIntOr(vm, 4, 1);
            int stepFast = (int)getIntOr(vm, 5, 100);
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 6, 0);
            int v = (int)value;
            ImGui::InputInt(label, &v, step, stepFast, flags);
            sq_pushinteger(vm, v);
            return 1;
        }

        static const size_t INPUT_TEXT_BUFFER_SIZE = 8192;
        static char inputTextBuffer[INPUT_TEXT_BUFFER_SIZE];

        inline void populateInputTextBuffer(const SQChar* str){
            strncpy(inputTextBuffer, str, INPUT_TEXT_BUFFER_SIZE - 1);
            inputTextBuffer[INPUT_TEXT_BUFFER_SIZE - 1] = '\0';
        }

        SQInteger inputText(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            const SQChar* value;
            sq_getstring(vm, 2, &label);
            sq_getstring(vm, 3, &value);
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 4, 0);
            populateInputTextBuffer(value);
            ImGui::InputText(label, inputTextBuffer, INPUT_TEXT_BUFFER_SIZE, flags);
            sq_pushstring(vm, inputTextBuffer, -1);
            return 1;
        }
        SQInteger inputTextMultiline(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            const SQChar* value;
            sq_getstring(vm, 2, &label);
            sq_getstring(vm, 3, &value);
            float w = (float)getFloatOr(vm, 4, 0.0f);
            float h = (float)getFloatOr(vm, 5, 0.0f);
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 6, 0);
            populateInputTextBuffer(value);
            ImGui::InputTextMultiline(label, inputTextBuffer, INPUT_TEXT_BUFFER_SIZE, ImVec2(w, h), flags);
            sq_pushstring(vm, inputTextBuffer, -1);
            return 1;
        }
        SQInteger colorEdit3(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            float col[3] = {0, 0, 0};
            readFloatArray(vm, 3, col, 3);
            ImGuiColorEditFlags flags = (ImGuiColorEditFlags)getIntOr(vm, 4, 0);
            bool changed = ImGui::ColorEdit3(label, col, flags);
            if(changed) writeFloatArray(vm, 3, col, 3);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger colorEdit4(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            float col[4] = {0, 0, 0, 1};
            readFloatArray(vm, 3, col, 4);
            ImGuiColorEditFlags flags = (ImGuiColorEditFlags)getIntOr(vm, 4, 0);
            bool changed = ImGui::ColorEdit4(label, col, flags);
            if(changed) writeFloatArray(vm, 3, col, 4);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger combo(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQInteger current;
            sq_getinteger(vm, 3, &current);
            std::vector<std::string> storage;
            std::vector<const char*> items;
            readStringArray(vm, 4, storage, items);
            int popupMaxHeight = (int)getIntOr(vm, 5, -1);
            int v = (int)current;
            ImGui::Combo(label, &v, items.data(), (int)items.size(), popupMaxHeight);
            sq_pushinteger(vm, v);
            return 1;
        }
        SQInteger listBox(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            SQInteger current;
            sq_getinteger(vm, 3, &current);
            std::vector<std::string> storage;
            std::vector<const char*> items;
            readStringArray(vm, 4, storage, items);
            int heightInItems = (int)getIntOr(vm, 5, -1);
            int v = (int)current;
            ImGui::ListBox(label, &v, items.data(), (int)items.size(), heightInItems);
            sq_pushinteger(vm, v);
            return 1;
        }

        //---------------------------------------------------------------------
        //Trees, headers
        //---------------------------------------------------------------------
        SQInteger treeNode(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            sq_pushbool(vm, ImGui::TreeNode(label));
            return 1;
        }
        SQInteger treeNodeEx(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::TreeNodeEx(label, flags));
            return 1;
        }
        SQInteger treePop(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::TreePop();
            return 0;
        }
        SQInteger collapsingHeader(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::CollapsingHeader(label, flags));
            return 1;
        }
        SQInteger setNextItemOpen(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQBool open;
            sq_getbool(vm, 2, &open);
            ImGuiCond cond = (ImGuiCond)getIntOr(vm, 3, 0);
            ImGui::SetNextItemOpen(open != SQFalse, cond);
            return 0;
        }

        //---------------------------------------------------------------------
        //Layout
        //---------------------------------------------------------------------
        SQInteger sameLine(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            float offsetX = (float)getFloatOr(vm, 2, 0.0f);
            float spacing = (float)getFloatOr(vm, 3, -1.0f);
            ImGui::SameLine(offsetX, spacing);
            return 0;
        }
        SQInteger newLine(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::NewLine();
            return 0;
        }
        SQInteger spacing(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::Spacing();
            return 0;
        }
        SQInteger separator(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::Separator();
            return 0;
        }
        SQInteger dummy(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat w, h;
            sq_getfloat(vm, 2, &w);
            sq_getfloat(vm, 3, &h);
            ImGui::Dummy(ImVec2((float)w, (float)h));
            return 0;
        }
        SQInteger indent(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::Indent((float)getFloatOr(vm, 2, 0.0f));
            return 0;
        }
        SQInteger unindent(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::Unindent((float)getFloatOr(vm, 2, 0.0f));
            return 0;
        }
        SQInteger beginGroup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::BeginGroup();
            return 0;
        }
        SQInteger endGroup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndGroup();
            return 0;
        }
        SQInteger alignTextToFramePadding(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::AlignTextToFramePadding();
            return 0;
        }
        SQInteger setNextItemWidth(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat w;
            sq_getfloat(vm, 2, &w);
            ImGui::SetNextItemWidth((float)w);
            return 0;
        }
        SQInteger pushItemWidth(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat w;
            sq_getfloat(vm, 2, &w);
            ImGui::PushItemWidth((float)w);
            return 0;
        }
        SQInteger popItemWidth(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::PopItemWidth();
            return 0;
        }
        SQInteger getCursorPosX(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetCursorPosX());
            return 1;
        }
        SQInteger getCursorPosY(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetCursorPosY());
            return 1;
        }
        SQInteger setCursorPos(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQFloat x, y;
            sq_getfloat(vm, 2, &x);
            sq_getfloat(vm, 3, &y);
            ImGui::SetCursorPos(ImVec2((float)x, (float)y));
            return 0;
        }
        SQInteger getCursorScreenPos(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const ImVec2 v = ImGui::GetCursorScreenPos();
            return pushVec2(vm, v.x, v.y);
        }
        SQInteger calcTextSize(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            float wrapWidth = (float)getFloatOr(vm, 3, -1.0f);
            const ImVec2 v = ImGui::CalcTextSize(str, 0, false, wrapWidth);
            return pushVec2(vm, v.x, v.y);
        }
        SQInteger getFrameHeight(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushfloat(vm, ImGui::GetFrameHeight());
            return 1;
        }
        SQInteger beginDisabled(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::BeginDisabled(getBoolOr(vm, 2, true));
            return 0;
        }
        SQInteger endDisabled(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndDisabled();
            return 0;
        }

        //---------------------------------------------------------------------
        //Menus, tabs
        //---------------------------------------------------------------------
        SQInteger beginMenuBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::BeginMenuBar());
            return 1;
        }
        SQInteger endMenuBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndMenuBar();
            return 0;
        }
        SQInteger beginMainMenuBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::BeginMainMenuBar());
            return 1;
        }
        SQInteger endMainMenuBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndMainMenuBar();
            return 0;
        }
        SQInteger beginMenu(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            bool enabled = getBoolOr(vm, 3, true);
            sq_pushbool(vm, ImGui::BeginMenu(label, enabled));
            return 1;
        }
        SQInteger endMenu(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndMenu();
            return 0;
        }
        SQInteger menuItem(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            const SQChar* shortcut = getStringOr(vm, 3, 0);
            bool selected = getBoolOr(vm, 4, false);
            bool enabled = getBoolOr(vm, 5, true);
            sq_pushbool(vm, ImGui::MenuItem(label, shortcut, selected, enabled));
            return 1;
        }
        SQInteger beginTabBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            ImGuiTabBarFlags flags = (ImGuiTabBarFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::BeginTabBar(id, flags));
            return 1;
        }
        SQInteger endTabBar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndTabBar();
            return 0;
        }
        SQInteger beginTabItem(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            ImGuiTabItemFlags flags = (ImGuiTabItemFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::BeginTabItem(label, 0, flags));
            return 1;
        }
        SQInteger endTabItem(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndTabItem();
            return 0;
        }

        //---------------------------------------------------------------------
        //Tables
        //---------------------------------------------------------------------
        SQInteger beginTable(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            SQInteger columns;
            sq_getinteger(vm, 3, &columns);
            ImGuiTableFlags flags = (ImGuiTableFlags)getIntOr(vm, 4, 0);
            float w = (float)getFloatOr(vm, 5, 0.0f);
            float h = (float)getFloatOr(vm, 6, 0.0f);
            sq_pushbool(vm, ImGui::BeginTable(id, (int)columns, flags, ImVec2(w, h)));
            return 1;
        }
        SQInteger endTable(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndTable();
            return 0;
        }
        SQInteger tableNextRow(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGuiTableRowFlags flags = (ImGuiTableRowFlags)getIntOr(vm, 2, 0);
            float minRowHeight = (float)getFloatOr(vm, 3, 0.0f);
            ImGui::TableNextRow(flags, minRowHeight);
            return 0;
        }
        SQInteger tableNextColumn(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::TableNextColumn());
            return 1;
        }
        SQInteger tableSetColumnIndex(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQInteger idx;
            sq_getinteger(vm, 2, &idx);
            sq_pushbool(vm, ImGui::TableSetColumnIndex((int)idx));
            return 1;
        }
        SQInteger tableSetupColumn(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            ImGuiTableColumnFlags flags = (ImGuiTableColumnFlags)getIntOr(vm, 3, 0);
            float initWidthOrWeight = (float)getFloatOr(vm, 4, 0.0f);
            ImGui::TableSetupColumn(label, flags, initWidthOrWeight);
            return 0;
        }
        SQInteger tableHeadersRow(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::TableHeadersRow();
            return 0;
        }

        //---------------------------------------------------------------------
        //Popups, tooltips
        //---------------------------------------------------------------------
        SQInteger openPopup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            ImGuiPopupFlags flags = (ImGuiPopupFlags)getIntOr(vm, 3, 0);
            ImGui::OpenPopup(id, flags);
            return 0;
        }
        SQInteger beginPopup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            ImGuiWindowFlags flags = (ImGuiWindowFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::BeginPopup(id, flags));
            return 1;
        }
        SQInteger beginPopupModal(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* name;
            sq_getstring(vm, 2, &name);
            ImGuiWindowFlags flags = (ImGuiWindowFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::BeginPopupModal(name, 0, flags));
            return 1;
        }
        SQInteger endPopup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndPopup();
            return 0;
        }
        SQInteger closeCurrentPopup(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::CloseCurrentPopup();
            return 0;
        }
        SQInteger isPopupOpen(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);
            ImGuiPopupFlags flags = (ImGuiPopupFlags)getIntOr(vm, 3, 0);
            sq_pushbool(vm, ImGui::IsPopupOpen(id, flags));
            return 1;
        }
        SQInteger beginTooltip(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::BeginTooltip());
            return 1;
        }
        SQInteger beginItemTooltip(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::BeginItemTooltip());
            return 1;
        }
        SQInteger endTooltip(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::EndTooltip();
            return 0;
        }
        SQInteger setTooltip(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            ImGui::SetTooltip("%s", str);
            return 0;
        }

        //---------------------------------------------------------------------
        //Plots
        //---------------------------------------------------------------------
        SQInteger plotValues(HSQUIRRELVM vm, bool histogram){
            const SQChar* label;
            sq_getstring(vm, 2, &label);

            SQInteger size = sq_getsize(vm, 3);
            std::vector<float> values;
            values.resize(size);
            readFloatArray(vm, 3, values.data(), (int)size);

            const SQChar* overlay = getStringOr(vm, 4, 0);
            float scaleMin = (float)getFloatOr(vm, 5, FLT_MAX);
            float scaleMax = (float)getFloatOr(vm, 6, FLT_MAX);
            float w = (float)getFloatOr(vm, 7, 0.0f);
            float h = (float)getFloatOr(vm, 8, 0.0f);

            if(histogram){
                ImGui::PlotHistogram(label, values.data(), (int)values.size(), 0, overlay, scaleMin, scaleMax, ImVec2(w, h));
            }else{
                ImGui::PlotLines(label, values.data(), (int)values.size(), 0, overlay, scaleMin, scaleMax, ImVec2(w, h));
            }
            return 0;
        }
        SQInteger plotLines(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            return plotValues(vm, false);
        }
        SQInteger plotHistogram(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            return plotValues(vm, true);
        }

        //---------------------------------------------------------------------
        //Item queries
        //---------------------------------------------------------------------
        SQInteger isItemHovered(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemHovered((ImGuiHoveredFlags)getIntOr(vm, 2, 0)));
            return 1;
        }
        SQInteger isItemActive(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemActive());
            return 1;
        }
        SQInteger isItemClicked(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemClicked((ImGuiMouseButton)getIntOr(vm, 2, 0)));
            return 1;
        }
        SQInteger isItemEdited(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemEdited());
            return 1;
        }
        SQInteger isItemActivated(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemActivated());
            return 1;
        }
        SQInteger isItemDeactivated(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemDeactivated());
            return 1;
        }
        SQInteger isItemDeactivatedAfterEdit(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsItemDeactivatedAfterEdit());
            return 1;
        }
        SQInteger isAnyItemActive(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsAnyItemActive());
            return 1;
        }
        SQInteger isAnyItemHovered(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsAnyItemHovered());
            return 1;
        }
        SQInteger setItemDefaultFocus(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::SetItemDefaultFocus();
            return 0;
        }
        SQInteger setKeyboardFocusHere(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::SetKeyboardFocusHere((int)getIntOr(vm, 2, 0));
            return 0;
        }

        //---------------------------------------------------------------------
        //ID stack, style
        //---------------------------------------------------------------------
        SQInteger pushId(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            if(sq_gettype(vm, 2) == OT_STRING){
                const SQChar* id;
                sq_getstring(vm, 2, &id);
                ImGui::PushID(id);
            }else{
                SQInteger id;
                sq_getinteger(vm, 2, &id);
                ImGui::PushID((int)id);
            }
            return 0;
        }
        SQInteger popId(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::PopID();
            return 0;
        }
        SQInteger pushStyleColor(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQInteger idx;
            sq_getinteger(vm, 2, &idx);
            SQFloat r, g, b, a;
            sq_getfloat(vm, 3, &r);
            sq_getfloat(vm, 4, &g);
            sq_getfloat(vm, 5, &b);
            sq_getfloat(vm, 6, &a);
            ImGui::PushStyleColor((ImGuiCol)idx, ImVec4((float)r, (float)g, (float)b, (float)a));
            return 0;
        }
        SQInteger popStyleColor(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::PopStyleColor((int)getIntOr(vm, 2, 1));
            return 0;
        }
        SQInteger pushStyleVar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            SQInteger idx;
            sq_getinteger(vm, 2, &idx);
            SQFloat a;
            sq_getfloat(vm, 3, &a);
            if(sq_gettop(vm) >= 4){
                SQFloat b;
                sq_getfloat(vm, 4, &b);
                ImGui::PushStyleVar((ImGuiStyleVar)idx, ImVec2((float)a, (float)b));
            }else{
                ImGui::PushStyleVar((ImGuiStyleVar)idx, (float)a);
            }
            return 0;
        }
        SQInteger popStyleVar(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGui::PopStyleVar((int)getIntOr(vm, 2, 1));
            return 0;
        }

        #undef IMGUI_FRAME_GUARD
    }

    void ImguiNamespace::setupNamespace(HSQUIRRELVM vm){
        //Demo, misc
        AV::ScriptUtils::addFunction(vm, showDemoWindow, "showDemoWindow");
        AV::ScriptUtils::addFunction(vm, showMetricsWindow, "showMetricsWindow");
        AV::ScriptUtils::addFunction(vm, getVersion, "getVersion");
        AV::ScriptUtils::addFunction(vm, getTime, "getTime");
        AV::ScriptUtils::addFunction(vm, getFrameCount, "getFrameCount");
        AV::ScriptUtils::addFunction(vm, getFramerate, "getFramerate");
        AV::ScriptUtils::addFunction(vm, getDeltaTime, "getDeltaTime");
        AV::ScriptUtils::addFunction(vm, getDisplaySize, "getDisplaySize");
        AV::ScriptUtils::addFunction(vm, wantCaptureMouse, "wantCaptureMouse");
        AV::ScriptUtils::addFunction(vm, wantCaptureKeyboard, "wantCaptureKeyboard");
        AV::ScriptUtils::addFunction(vm, wantTextInput, "wantTextInput");
        AV::ScriptUtils::addFunction(vm, isFirstUpdateOfFrame, "isFirstUpdateOfFrame");
        AV::ScriptUtils::addFunction(vm, setRenderingEnabled, "setRenderingEnabled", 2, ".b");
        AV::ScriptUtils::addFunction(vm, createOverlayWorkspace, "createOverlayWorkspace");
        AV::ScriptUtils::addFunction(vm, destroyOverlayWorkspace, "destroyOverlayWorkspace");
        AV::ScriptUtils::addFunction(vm, setAutoOverlayEnabled, "setAutoOverlayEnabled", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getAutoOverlayEnabled, "getAutoOverlayEnabled");

        //Windows
        AV::ScriptUtils::addFunction(vm, begin, "begin", -2, ".si");
        AV::ScriptUtils::addFunction(vm, end, "end");
        AV::ScriptUtils::addFunction(vm, beginChild, "beginChild", -2, ".snnii");
        AV::ScriptUtils::addFunction(vm, endChild, "endChild");
        AV::ScriptUtils::addFunction(vm, setNextWindowPos, "setNextWindowPos", -3, ".nninn");
        AV::ScriptUtils::addFunction(vm, setNextWindowSize, "setNextWindowSize", -3, ".nni");
        AV::ScriptUtils::addFunction(vm, setNextWindowCollapsed, "setNextWindowCollapsed", -2, ".bi");
        AV::ScriptUtils::addFunction(vm, setNextWindowFocus, "setNextWindowFocus");
        AV::ScriptUtils::addFunction(vm, setNextWindowBgAlpha, "setNextWindowBgAlpha", 2, ".n");
        AV::ScriptUtils::addFunction(vm, getWindowPos, "getWindowPos");
        AV::ScriptUtils::addFunction(vm, getWindowSize, "getWindowSize");
        AV::ScriptUtils::addFunction(vm, getWindowWidth, "getWindowWidth");
        AV::ScriptUtils::addFunction(vm, getWindowHeight, "getWindowHeight");
        AV::ScriptUtils::addFunction(vm, getContentRegionAvail, "getContentRegionAvail");
        AV::ScriptUtils::addFunction(vm, isWindowHovered, "isWindowHovered", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isWindowFocused, "isWindowFocused", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isWindowCollapsed, "isWindowCollapsed");

        //Text
        AV::ScriptUtils::addFunction(vm, text, "text", 2, ".s");
        AV::ScriptUtils::addFunction(vm, textColored, "textColored", 6, ".nnnns");
        AV::ScriptUtils::addFunction(vm, textDisabled, "textDisabled", 2, ".s");
        AV::ScriptUtils::addFunction(vm, textWrapped, "textWrapped", 2, ".s");
        AV::ScriptUtils::addFunction(vm, bulletText, "bulletText", 2, ".s");
        AV::ScriptUtils::addFunction(vm, labelText, "labelText", 3, ".ss");
        AV::ScriptUtils::addFunction(vm, separatorText, "separatorText", 2, ".s");

        //Main widgets
        AV::ScriptUtils::addFunction(vm, button, "button", -2, ".snn");
        AV::ScriptUtils::addFunction(vm, smallButton, "smallButton", 2, ".s");
        AV::ScriptUtils::addFunction(vm, invisibleButton, "invisibleButton", -4, ".snni");
        AV::ScriptUtils::addFunction(vm, arrowButton, "arrowButton", 3, ".si");
        AV::ScriptUtils::addFunction(vm, checkbox, "checkbox", 3, ".sb");
        AV::ScriptUtils::addFunction(vm, radioButton, "radioButton", 3, ".sb");
        AV::ScriptUtils::addFunction(vm, selectable, "selectable", -2, ".sbinn");
        AV::ScriptUtils::addFunction(vm, progressBar, "progressBar", -2, ".nnns|o");
        AV::ScriptUtils::addFunction(vm, bullet, "bullet");

        //Value widgets
        AV::ScriptUtils::addFunction(vm, sliderFloat, "sliderFloat", -5, ".snnns|oi");
        AV::ScriptUtils::addFunction(vm, sliderInt, "sliderInt", -5, ".siiis|oi");
        AV::ScriptUtils::addFunction(vm, sliderFloatN, "sliderFloatN", -5, ".sanns|oi");
        AV::ScriptUtils::addFunction(vm, dragFloat, "dragFloat", -3, ".snnnns|oi");
        AV::ScriptUtils::addFunction(vm, dragInt, "dragInt", -3, ".sinnns|oi");
        AV::ScriptUtils::addFunction(vm, inputFloat, "inputFloat", -3, ".snnns|oi");
        AV::ScriptUtils::addFunction(vm, inputInt, "inputInt", -3, ".siiii");
        AV::ScriptUtils::addFunction(vm, inputText, "inputText", -3, ".ssi");
        AV::ScriptUtils::addFunction(vm, inputTextMultiline, "inputTextMultiline", -3, ".ssnni");
        AV::ScriptUtils::addFunction(vm, colorEdit3, "colorEdit3", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, colorEdit4, "colorEdit4", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, combo, "combo", -4, ".siai");
        AV::ScriptUtils::addFunction(vm, listBox, "listBox", -4, ".siai");

        //Trees
        AV::ScriptUtils::addFunction(vm, treeNode, "treeNode", 2, ".s");
        AV::ScriptUtils::addFunction(vm, treeNodeEx, "treeNodeEx", -2, ".si");
        AV::ScriptUtils::addFunction(vm, treePop, "treePop");
        AV::ScriptUtils::addFunction(vm, collapsingHeader, "collapsingHeader", -2, ".si");
        AV::ScriptUtils::addFunction(vm, setNextItemOpen, "setNextItemOpen", -2, ".bi");

        //Layout
        AV::ScriptUtils::addFunction(vm, sameLine, "sameLine", -1, ".nn");
        AV::ScriptUtils::addFunction(vm, newLine, "newLine");
        AV::ScriptUtils::addFunction(vm, spacing, "spacing");
        AV::ScriptUtils::addFunction(vm, separator, "separator");
        AV::ScriptUtils::addFunction(vm, dummy, "dummy", 3, ".nn");
        AV::ScriptUtils::addFunction(vm, indent, "indent", -1, ".n");
        AV::ScriptUtils::addFunction(vm, unindent, "unindent", -1, ".n");
        AV::ScriptUtils::addFunction(vm, beginGroup, "beginGroup");
        AV::ScriptUtils::addFunction(vm, endGroup, "endGroup");
        AV::ScriptUtils::addFunction(vm, alignTextToFramePadding, "alignTextToFramePadding");
        AV::ScriptUtils::addFunction(vm, setNextItemWidth, "setNextItemWidth", 2, ".n");
        AV::ScriptUtils::addFunction(vm, pushItemWidth, "pushItemWidth", 2, ".n");
        AV::ScriptUtils::addFunction(vm, popItemWidth, "popItemWidth");
        AV::ScriptUtils::addFunction(vm, getCursorPosX, "getCursorPosX");
        AV::ScriptUtils::addFunction(vm, getCursorPosY, "getCursorPosY");
        AV::ScriptUtils::addFunction(vm, setCursorPos, "setCursorPos", 3, ".nn");
        AV::ScriptUtils::addFunction(vm, getCursorScreenPos, "getCursorScreenPos");
        AV::ScriptUtils::addFunction(vm, calcTextSize, "calcTextSize", -2, ".sn");
        AV::ScriptUtils::addFunction(vm, getFrameHeight, "getFrameHeight");
        AV::ScriptUtils::addFunction(vm, beginDisabled, "beginDisabled", -1, ".b");
        AV::ScriptUtils::addFunction(vm, endDisabled, "endDisabled");

        //Menus, tabs
        AV::ScriptUtils::addFunction(vm, beginMenuBar, "beginMenuBar");
        AV::ScriptUtils::addFunction(vm, endMenuBar, "endMenuBar");
        AV::ScriptUtils::addFunction(vm, beginMainMenuBar, "beginMainMenuBar");
        AV::ScriptUtils::addFunction(vm, endMainMenuBar, "endMainMenuBar");
        AV::ScriptUtils::addFunction(vm, beginMenu, "beginMenu", -2, ".sb");
        AV::ScriptUtils::addFunction(vm, endMenu, "endMenu");
        AV::ScriptUtils::addFunction(vm, menuItem, "menuItem", -2, ".ss|obb");
        AV::ScriptUtils::addFunction(vm, beginTabBar, "beginTabBar", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endTabBar, "endTabBar");
        AV::ScriptUtils::addFunction(vm, beginTabItem, "beginTabItem", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endTabItem, "endTabItem");

        //Tables
        AV::ScriptUtils::addFunction(vm, beginTable, "beginTable", -3, ".siinn");
        AV::ScriptUtils::addFunction(vm, endTable, "endTable");
        AV::ScriptUtils::addFunction(vm, tableNextRow, "tableNextRow", -1, ".in");
        AV::ScriptUtils::addFunction(vm, tableNextColumn, "tableNextColumn");
        AV::ScriptUtils::addFunction(vm, tableSetColumnIndex, "tableSetColumnIndex", 2, ".i");
        AV::ScriptUtils::addFunction(vm, tableSetupColumn, "tableSetupColumn", -2, ".sin");
        AV::ScriptUtils::addFunction(vm, tableHeadersRow, "tableHeadersRow");

        //Popups, tooltips
        AV::ScriptUtils::addFunction(vm, openPopup, "openPopup", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginPopup, "beginPopup", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginPopupModal, "beginPopupModal", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endPopup, "endPopup");
        AV::ScriptUtils::addFunction(vm, closeCurrentPopup, "closeCurrentPopup");
        AV::ScriptUtils::addFunction(vm, isPopupOpen, "isPopupOpen", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginTooltip, "beginTooltip");
        AV::ScriptUtils::addFunction(vm, beginItemTooltip, "beginItemTooltip");
        AV::ScriptUtils::addFunction(vm, endTooltip, "endTooltip");
        AV::ScriptUtils::addFunction(vm, setTooltip, "setTooltip", 2, ".s");

        //Plots
        AV::ScriptUtils::addFunction(vm, plotLines, "plotLines", -3, ".sas|onnnn");
        AV::ScriptUtils::addFunction(vm, plotHistogram, "plotHistogram", -3, ".sas|onnnn");

        //Item queries
        AV::ScriptUtils::addFunction(vm, isItemHovered, "isItemHovered", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isItemActive, "isItemActive");
        AV::ScriptUtils::addFunction(vm, isItemClicked, "isItemClicked", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isItemEdited, "isItemEdited");
        AV::ScriptUtils::addFunction(vm, isItemActivated, "isItemActivated");
        AV::ScriptUtils::addFunction(vm, isItemDeactivated, "isItemDeactivated");
        AV::ScriptUtils::addFunction(vm, isItemDeactivatedAfterEdit, "isItemDeactivatedAfterEdit");
        AV::ScriptUtils::addFunction(vm, isAnyItemActive, "isAnyItemActive");
        AV::ScriptUtils::addFunction(vm, isAnyItemHovered, "isAnyItemHovered");
        AV::ScriptUtils::addFunction(vm, setItemDefaultFocus, "setItemDefaultFocus");
        AV::ScriptUtils::addFunction(vm, setKeyboardFocusHere, "setKeyboardFocusHere", -1, ".i");

        //ID, style
        AV::ScriptUtils::addFunction(vm, pushId, "pushId", 2, ".s|i");
        AV::ScriptUtils::addFunction(vm, popId, "popId");
        AV::ScriptUtils::addFunction(vm, pushStyleColor, "pushStyleColor", 6, ".innnn");
        AV::ScriptUtils::addFunction(vm, popStyleColor, "popStyleColor", -1, ".i");
        AV::ScriptUtils::addFunction(vm, pushStyleVar, "pushStyleVar", -3, ".inn");
        AV::ScriptUtils::addFunction(vm, popStyleVar, "popStyleVar", -1, ".i");

        _setupConstants(vm);
    }
}
