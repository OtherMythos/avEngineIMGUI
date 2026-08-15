#include "ImguiNamespace.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "Scripting/ScriptNamespace/Classes/ColourValueUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

#include "OgreTextureGpu.h"

#include "imgui.h"
//The dock builder lives in the internal api, which is where imgui keeps it
//until the layout functions settle down.
#include "imgui_internal.h"

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
                sq_rawset(vm, idx);
            }
        }

        //Read up to maxCount integers from the array at idx. Returns the count read.
        inline int readIntArray(HSQUIRRELVM vm, SQInteger idx, int* out, int maxCount){
            SQInteger size = sq_getsize(vm, idx);
            int count = (int)(size < maxCount ? size : maxCount);
            for(int i = 0; i < count; i++){
                sq_pushinteger(vm, i);
                if(SQ_FAILED(sq_get(vm, idx))) return i;
                SQInteger v = 0;
                sq_getinteger(vm, -1, &v);
                sq_pop(vm, 1);
                out[i] = (int)v;
            }
            return count;
        }

        //Write integers back into the array at idx.
        inline void writeIntArray(HSQUIRRELVM vm, SQInteger idx, const int* vals, int count){
            for(int i = 0; i < count; i++){
                sq_pushinteger(vm, i);
                sq_pushinteger(vm, vals[i]);
                sq_rawset(vm, idx);
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
        SQInteger setGlobalScale(HSQUIRRELVM vm){
            //Deliberately does not begin the frame. The scale is applied as the
            //next frame begins, so beginning one here would mean the gui this
            //update is about to build was the last one drawn at the old scale.
            SQFloat scale;
            sq_getfloat(vm, 2, &scale);
            ImguiManager* manager = ImguiManager::getSingletonPtr();
            if(manager) manager->setGlobalScale(scale);
            return 0;
        }
        SQInteger getGlobalScale(HSQUIRRELVM vm){
            ImguiManager* manager = ImguiManager::getSingletonPtr();
            sq_pushfloat(vm, manager ? manager->getGlobalScale() : 1.0f);
            return 1;
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
        SQInteger beginClosable(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* name;
            sq_getstring(vm, 2, &name);
            ImGuiWindowFlags flags = (ImGuiWindowFlags)getIntOr(vm, 3, 0);

            //Handing imgui somewhere to write is what puts the close button in
            //the title bar, and on the window's tab once it is docked. Nothing
            //else turns it on - there is no window flag for a close button.
            //
            //Whether the window is open belongs to the script, so this starts
            //true every frame and reports back rather than remembering. imgui
            //only ever writes false into it, on the frame the button is clicked.
            bool open = true;
            bool visible = ImGui::Begin(name, &open, flags);

            //Both, because they answer different questions: whether to draw the
            //contents, and whether the window has just been closed.
            sq_newarray(vm, 0);
            sq_pushbool(vm, visible);
            sq_arrayappend(vm, -2);
            sq_pushbool(vm, open);
            sq_arrayappend(vm, -2);
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
        //Docking
        //
        //From the docking branch of imgui. Docking happens entirely inside the
        //engine's render target: multi viewport (imgui windows in real OS
        //windows) is never enabled, as the plugin has no platform backend.
        //@see ImguiManager::createFontTexture
        //---------------------------------------------------------------------
        inline bool dockingEnabled(){
            return (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0;
        }

        //A dock id is either a string, hashed the way imgui hashes any id, or
        //an integer previously returned by dockSpace, dockSpaceOverViewport or
        //getId. Strings are the convenient form; the integer form is what makes
        //an id from one call usable in another.
        inline ImGuiID readDockId(HSQUIRRELVM vm, SQInteger idx){
            if(sq_gettype(vm, idx) == OT_STRING){
                const SQChar* str;
                sq_getstring(vm, idx, &str);
                return ImGui::GetID(str);
            }
            SQInteger value = 0;
            sq_getinteger(vm, idx, &value);
            return (ImGuiID)value;
        }

        SQInteger dockSpace(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            if(!dockingEnabled()){
                return sq_throwerror(vm, "dockSpace requires docking, which is disabled. Call setDockingEnabled(true) first.");
            }
            ImGuiID id = readDockId(vm, 2);
            //imgui asserts on a zero id, which would stop the engine.
            if(id == 0){
                return sq_throwerror(vm, "dockSpace requires a non-zero dock id.");
            }
            float w = (float)getFloatOr(vm, 3, 0.0f);
            float h = (float)getFloatOr(vm, 4, 0.0f);
            ImGuiDockNodeFlags flags = (ImGuiDockNodeFlags)getIntOr(vm, 5, 0);
            sq_pushinteger(vm, (SQInteger)ImGui::DockSpace(id, ImVec2(w, h), flags));
            return 1;
        }
        SQInteger dockSpaceOverViewport(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            if(!dockingEnabled()){
                return sq_throwerror(vm, "dockSpaceOverViewport requires docking, which is disabled. Call setDockingEnabled(true) first.");
            }
            //Optional id first, so dockSpaceOverViewport() alone is the common
            //case: imgui then derives its own id.
            ImGuiID id = 0;
            if(sq_gettop(vm) >= 2 && sq_gettype(vm, 2) != OT_NULL){
                id = readDockId(vm, 2);
            }
            ImGuiDockNodeFlags flags = (ImGuiDockNodeFlags)getIntOr(vm, 3, 0);
            //The viewport is always the main one, there being only one.
            sq_pushinteger(vm, (SQInteger)ImGui::DockSpaceOverViewport(id, ImGui::GetMainViewport(), flags));
            return 1;
        }
        SQInteger setNextWindowDockId(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            ImGuiID id = readDockId(vm, 2);
            ImGuiCond cond = (ImGuiCond)getIntOr(vm, 3, 0);
            ImGui::SetNextWindowDockID(id, cond);
            return 0;
        }
        SQInteger getWindowDockId(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushinteger(vm, (SQInteger)ImGui::GetWindowDockID());
            return 1;
        }
        SQInteger isWindowDocked(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            sq_pushbool(vm, ImGui::IsWindowDocked());
            return 1;
        }
        SQInteger setDockingEnabled(HSQUIRRELVM vm){
            SQBool enabled;
            sq_getbool(vm, 2, &enabled);
            ImGuiIO& io = ImGui::GetIO();
            if(enabled != SQFalse) io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            else io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
            return 0;
        }
        SQInteger getDockingEnabled(HSQUIRRELVM vm){
            sq_pushbool(vm, dockingEnabled());
            return 1;
        }
        SQInteger setDockingNoSplit(HSQUIRRELVM vm){
            SQBool value;
            sq_getbool(vm, 2, &value);
            ImGui::GetIO().ConfigDockingNoSplit = value != SQFalse;
            return 0;
        }
        SQInteger getDockingNoSplit(HSQUIRRELVM vm){
            sq_pushbool(vm, ImGui::GetIO().ConfigDockingNoSplit);
            return 1;
        }
        SQInteger setDockingWithShift(HSQUIRRELVM vm){
            SQBool value;
            sq_getbool(vm, 2, &value);
            ImGui::GetIO().ConfigDockingWithShift = value != SQFalse;
            return 0;
        }
        SQInteger getDockingWithShift(HSQUIRRELVM vm){
            sq_pushbool(vm, ImGui::GetIO().ConfigDockingWithShift);
            return 1;
        }
        //The dock builder. A dockspace on its own gives every window the same
        //node, so they arrive as tabs on top of each other; describing a layout
        //- this panel left, that one right, the rest in the middle - means
        //building the nodes up front and docking windows into them by name.
        //
        //A layout is built once, not every frame, and the windows it names are
        //docked whether or not they have been submitted yet.
        SQInteger dockBuilderAddNode(HSQUIRRELVM vm){
            ImGuiID id = sq_gettop(vm) >= 2 && sq_gettype(vm, 2) != OT_NULL ? readDockId(vm, 2) : 0;
            ImGuiDockNodeFlags flags = (ImGuiDockNodeFlags)getIntOr(vm, 3, 0);
            //Adding a node with the DockSpace flag creates a dockspace, and a
            //dockspace must be named. Zero is fine for a plain node, where it
            //means 'any id will do', but here imgui asserts and stops the
            //engine rather than returning.
            if(id == 0 && (flags & ImGuiDockNodeFlags_DockSpace)){
                return sq_throwerror(vm, "dockBuilderAddNode requires a non-zero id when DockNodeFlags_DockSpace is set.");
            }
            sq_pushinteger(vm, (SQInteger)ImGui::DockBuilderAddNode(id, flags));
            return 1;
        }
        SQInteger dockBuilderRemoveNode(HSQUIRRELVM vm){
            ImGui::DockBuilderRemoveNode(readDockId(vm, 2));
            return 0;
        }
        SQInteger dockBuilderSetNodeSize(HSQUIRRELVM vm){
            ImGuiID id = readDockId(vm, 2);
            SQFloat w, h;
            sq_getfloat(vm, 3, &w);
            sq_getfloat(vm, 4, &h);
            ImGui::DockBuilderSetNodeSize(id, ImVec2((float)w, (float)h));
            return 0;
        }
        SQInteger dockBuilderSplitNode(HSQUIRRELVM vm){
            ImGuiID id = readDockId(vm, 2);
            SQInteger dir;
            sq_getinteger(vm, 3, &dir);
            SQFloat ratio;
            sq_getfloat(vm, 4, &ratio);

            ImGuiID atDir = 0;
            ImGuiID atOpposite = 0;
            ImGui::DockBuilderSplitNode(id, (ImGuiDir)dir, (float)ratio, &atDir, &atOpposite);

            //Both halves are returned because splitting replaces the node: the
            //id which was split is now their parent and cannot be docked into.
            sq_newarray(vm, 0);
            sq_pushinteger(vm, (SQInteger)atDir);
            sq_arrayappend(vm, -2);
            sq_pushinteger(vm, (SQInteger)atOpposite);
            sq_arrayappend(vm, -2);
            return 1;
        }
        SQInteger dockBuilderDockWindow(HSQUIRRELVM vm){
            const SQChar* windowName;
            sq_getstring(vm, 2, &windowName);
            ImGui::DockBuilderDockWindow(windowName, readDockId(vm, 3));
            return 0;
        }
        SQInteger dockBuilderFinish(HSQUIRRELVM vm){
            ImGui::DockBuilderFinish(readDockId(vm, 2));
            return 0;
        }

        SQInteger setDockingAlwaysTabBar(HSQUIRRELVM vm){
            SQBool value;
            sq_getbool(vm, 2, &value);
            ImGui::GetIO().ConfigDockingAlwaysTabBar = value != SQFalse;
            return 0;
        }
        SQInteger getDockingAlwaysTabBar(HSQUIRRELVM vm){
            sq_pushbool(vm, ImGui::GetIO().ConfigDockingAlwaysTabBar);
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
        //Images
        //---------------------------------------------------------------------
        //Read one of the engine's texture user datas off the stack. Returns 0
        //on success, or the result of sq_throwerror to be returned by the
        //caller, in the style of the engine's own namespaces.
        inline SQInteger readTexture(HSQUIRRELVM vm, SQInteger idx, Ogre::TextureGpu** outTexture){
            bool userOwned = false;
            bool isValid = false;
            AV::UserDataGetResult result = AV::TextureUserData::readTextureFromUserData(vm, idx, outTexture, &userOwned, &isValid);
            if(result != AV::USER_DATA_GET_SUCCESS){
                return sq_throwerror(vm, AV::ScriptUtils::checkResultErrorMessage(result));
            }
            //A texture destroyed while script still holds the user data. The
            //renderer would bind the dangling pointer, so this has to stop here.
            if(!isValid || !*outTexture){
                return sq_throwerror(vm, "The provided texture is no longer valid.");
            }
            return 0;
        }

        SQInteger image(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            Ogre::TextureGpu* texture = 0;
            SQInteger readResult = readTexture(vm, 2, &texture);
            if(readResult != 0) return readResult;

            SQFloat w, h;
            sq_getfloat(vm, 3, &w);
            sq_getfloat(vm, 4, &h);
            const ImVec2 size((float)w, (float)h);

            //Uvs are the whole texture unless given. Flipping them is how a
            //render target is drawn the right way up on an api which disagrees
            //with the one the texture was rendered on.
            const ImVec2 uv0((float)getFloatOr(vm, 5, 0.0f), (float)getFloatOr(vm, 6, 0.0f));
            const ImVec2 uv1((float)getFloatOr(vm, 7, 1.0f), (float)getFloatOr(vm, 8, 1.0f));

            //A texture which is not resident has nothing on the gpu to bind, so
            //the renderer must not be handed it. This is a normal transient
            //state while a texture streams in rather than a script error, so
            //the space is still taken up and the image appears once it arrives.
            if(texture->getResidencyStatus() != Ogre::GpuResidency::Resident){
                ImGui::Dummy(size);
                return 0;
            }

            //@see ImguiManager::render, which casts this straight back to an
            //Ogre::TextureGpu* and binds it for the draw call.
            ImGui::Image((ImTextureID)(uintptr_t)texture, size, uv0, uv1);
            return 0;
        }

        SQInteger imageButton(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* id;
            sq_getstring(vm, 2, &id);

            Ogre::TextureGpu* texture = 0;
            SQInteger readResult = readTexture(vm, 3, &texture);
            if(readResult != 0) return readResult;

            SQFloat w, h;
            sq_getfloat(vm, 4, &w);
            sq_getfloat(vm, 5, &h);
            const ImVec2 size((float)w, (float)h);
            const ImVec2 uv0((float)getFloatOr(vm, 6, 0.0f), (float)getFloatOr(vm, 7, 0.0f));
            const ImVec2 uv1((float)getFloatOr(vm, 8, 1.0f), (float)getFloatOr(vm, 9, 1.0f));

            Ogre::ColourValue background(0, 0, 0, 0);
            if(sq_gettop(vm) >= 10){
                AV::UserDataGetResult result = AV::ColourValueUserData::readColourValueFromUserData(vm, 10, &background);
                if(result != AV::USER_DATA_GET_SUCCESS){
                    return sq_throwerror(vm, AV::ScriptUtils::checkResultErrorMessage(result));
                }
            }

            Ogre::ColourValue tint = Ogre::ColourValue::White;
            if(sq_gettop(vm) >= 11){
                AV::UserDataGetResult result = AV::ColourValueUserData::readColourValueFromUserData(vm, 11, &tint);
                if(result != AV::USER_DATA_GET_SUCCESS){
                    return sq_throwerror(vm, AV::ScriptUtils::checkResultErrorMessage(result));
                }
            }

            const ImVec4 backgroundColour((float)background.r, (float)background.g,
                (float)background.b, (float)background.a);
            const ImVec4 tintColour((float)tint.r, (float)tint.g, (float)tint.b, (float)tint.a);
            sq_pushbool(vm, ImGui::ImageButton(id, (ImTextureID)(uintptr_t)texture,
                size, uv0, uv1, backgroundColour, tintColour));
            return 1;
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
            //Checked before reading, so an oversized array is an error rather
            //than being silently truncated to the first four elements.
            SQInteger arraySize = sq_getsize(vm, 3);
            if(arraySize < 1 || arraySize > 4){
                return sq_throwerror(vm, "sliderFloatN expects an array of 1 to 4 numbers.");
            }
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

        //The multi component drag and input widgets. Squirrel has no reference
        //primitives, so unlike their single value counterparts these take an
        //array which is mutated in place, and return whether it changed. Each
        //family differs only in the imgui call, so the argument handling lives
        //in one place and the bindings are thin wrappers over it.
        //The size is checked before reading, so an array of the wrong length is
        //an error rather than being silently truncated or partially read.
        SQInteger dragFloatArray(HSQUIRRELVM vm, int count, const SQChar* sizeError){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            if(sq_getsize(vm, 3) != count){
                return sq_throwerror(vm, sizeError);
            }
            float values[4];
            readFloatArray(vm, 3, values, count);
            float speed = (float)getFloatOr(vm, 4, 1.0f);
            float minVal = (float)getFloatOr(vm, 5, 0.0f);
            float maxVal = (float)getFloatOr(vm, 6, 0.0f);
            const SQChar* format = getStringOr(vm, 7, "%.3f");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 8, 0);

            bool changed = false;
            switch(count){
                case 2: changed = ImGui::DragFloat2(label, values, speed, minVal, maxVal, format, flags); break;
                case 3: changed = ImGui::DragFloat3(label, values, speed, minVal, maxVal, format, flags); break;
                case 4: changed = ImGui::DragFloat4(label, values, speed, minVal, maxVal, format, flags); break;
                default: return sq_throwerror(vm, sizeError);
            }
            if(changed) writeFloatArray(vm, 3, values, count);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger dragIntArray(HSQUIRRELVM vm, int count, const SQChar* sizeError){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            if(sq_getsize(vm, 3) != count){
                return sq_throwerror(vm, sizeError);
            }
            int values[4];
            readIntArray(vm, 3, values, count);
            float speed = (float)getFloatOr(vm, 4, 1.0f);
            int minVal = (int)getIntOr(vm, 5, 0);
            int maxVal = (int)getIntOr(vm, 6, 0);
            const SQChar* format = getStringOr(vm, 7, "%d");
            ImGuiSliderFlags flags = (ImGuiSliderFlags)getIntOr(vm, 8, 0);

            bool changed = false;
            switch(count){
                case 2: changed = ImGui::DragInt2(label, values, speed, minVal, maxVal, format, flags); break;
                case 3: changed = ImGui::DragInt3(label, values, speed, minVal, maxVal, format, flags); break;
                case 4: changed = ImGui::DragInt4(label, values, speed, minVal, maxVal, format, flags); break;
                default: return sq_throwerror(vm, sizeError);
            }
            if(changed) writeIntArray(vm, 3, values, count);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger inputFloatArray(HSQUIRRELVM vm, int count, const SQChar* sizeError){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            if(sq_getsize(vm, 3) != count){
                return sq_throwerror(vm, sizeError);
            }
            float values[4];
            readFloatArray(vm, 3, values, count);
            const SQChar* format = getStringOr(vm, 4, "%.3f");
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 5, 0);

            bool changed = false;
            switch(count){
                case 2: changed = ImGui::InputFloat2(label, values, format, flags); break;
                case 3: changed = ImGui::InputFloat3(label, values, format, flags); break;
                case 4: changed = ImGui::InputFloat4(label, values, format, flags); break;
                default: return sq_throwerror(vm, sizeError);
            }
            if(changed) writeFloatArray(vm, 3, values, count);
            sq_pushbool(vm, changed);
            return 1;
        }
        SQInteger inputIntArray(HSQUIRRELVM vm, int count, const SQChar* sizeError){
            IMGUI_FRAME_GUARD
            const SQChar* label;
            sq_getstring(vm, 2, &label);
            if(sq_getsize(vm, 3) != count){
                return sq_throwerror(vm, sizeError);
            }
            int values[4];
            readIntArray(vm, 3, values, count);
            ImGuiInputTextFlags flags = (ImGuiInputTextFlags)getIntOr(vm, 4, 0);

            bool changed = false;
            switch(count){
                case 2: changed = ImGui::InputInt2(label, values, flags); break;
                case 3: changed = ImGui::InputInt3(label, values, flags); break;
                case 4: changed = ImGui::InputInt4(label, values, flags); break;
                default: return sq_throwerror(vm, sizeError);
            }
            if(changed) writeIntArray(vm, 3, values, count);
            sq_pushbool(vm, changed);
            return 1;
        }

        SQInteger dragFloat2(HSQUIRRELVM vm){
            return dragFloatArray(vm, 2, "dragFloat2 expects an array of 2 numbers.");
        }
        SQInteger dragFloat3(HSQUIRRELVM vm){
            return dragFloatArray(vm, 3, "dragFloat3 expects an array of 3 numbers.");
        }
        SQInteger dragFloat4(HSQUIRRELVM vm){
            return dragFloatArray(vm, 4, "dragFloat4 expects an array of 4 numbers.");
        }
        SQInteger dragInt2(HSQUIRRELVM vm){
            return dragIntArray(vm, 2, "dragInt2 expects an array of 2 numbers.");
        }
        SQInteger dragInt3(HSQUIRRELVM vm){
            return dragIntArray(vm, 3, "dragInt3 expects an array of 3 numbers.");
        }
        SQInteger dragInt4(HSQUIRRELVM vm){
            return dragIntArray(vm, 4, "dragInt4 expects an array of 4 numbers.");
        }
        SQInteger inputFloat2(HSQUIRRELVM vm){
            return inputFloatArray(vm, 2, "inputFloat2 expects an array of 2 numbers.");
        }
        SQInteger inputFloat3(HSQUIRRELVM vm){
            return inputFloatArray(vm, 3, "inputFloat3 expects an array of 3 numbers.");
        }
        SQInteger inputFloat4(HSQUIRRELVM vm){
            return inputFloatArray(vm, 4, "inputFloat4 expects an array of 4 numbers.");
        }
        SQInteger inputInt2(HSQUIRRELVM vm){
            return inputIntArray(vm, 2, "inputInt2 expects an array of 2 numbers.");
        }
        SQInteger inputInt3(HSQUIRRELVM vm){
            return inputIntArray(vm, 3, "inputInt3 expects an array of 3 numbers.");
        }
        SQInteger inputInt4(HSQUIRRELVM vm){
            return inputIntArray(vm, 4, "inputInt4 expects an array of 4 numbers.");
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
            if(sq_getsize(vm, 3) < 3){
                return sq_throwerror(vm, "colorEdit3 expects an array of 3 numbers.");
            }
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
            if(sq_getsize(vm, 3) < 4){
                return sq_throwerror(vm, "colorEdit4 expects an array of 4 numbers.");
            }
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
        SQInteger getId(HSQUIRRELVM vm){
            IMGUI_FRAME_GUARD
            const SQChar* str;
            sq_getstring(vm, 2, &str);
            //Hashed against the current window and id stack, exactly as imgui
            //hashes a widget label, so the same string yields the same id only
            //in the same place.
            sq_pushinteger(vm, (SQInteger)ImGui::GetID(str));
            return 1;
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
        AV::ScriptUtils::addFunction(vm, showDemoWindow, "showDemoWindow", 1, ".");
        AV::ScriptUtils::addFunction(vm, showMetricsWindow, "showMetricsWindow", 1, ".");
        AV::ScriptUtils::addFunction(vm, getVersion, "getVersion", 1, ".");
        AV::ScriptUtils::addFunction(vm, getTime, "getTime", 1, ".");
        AV::ScriptUtils::addFunction(vm, getFrameCount, "getFrameCount", 1, ".");
        AV::ScriptUtils::addFunction(vm, getFramerate, "getFramerate", 1, ".");
        AV::ScriptUtils::addFunction(vm, getDeltaTime, "getDeltaTime", 1, ".");
        AV::ScriptUtils::addFunction(vm, getDisplaySize, "getDisplaySize", 1, ".");
        AV::ScriptUtils::addFunction(vm, wantCaptureMouse, "wantCaptureMouse", 1, ".");
        AV::ScriptUtils::addFunction(vm, wantCaptureKeyboard, "wantCaptureKeyboard", 1, ".");
        AV::ScriptUtils::addFunction(vm, wantTextInput, "wantTextInput", 1, ".");
        AV::ScriptUtils::addFunction(vm, isFirstUpdateOfFrame, "isFirstUpdateOfFrame", 1, ".");
        AV::ScriptUtils::addFunction(vm, setRenderingEnabled, "setRenderingEnabled", 2, ".b");
        AV::ScriptUtils::addFunction(vm, setGlobalScale, "setGlobalScale", 2, ".n");
        AV::ScriptUtils::addFunction(vm, getGlobalScale, "getGlobalScale", 1, ".");
        AV::ScriptUtils::addFunction(vm, createOverlayWorkspace, "createOverlayWorkspace", 1, ".");
        AV::ScriptUtils::addFunction(vm, destroyOverlayWorkspace, "destroyOverlayWorkspace", 1, ".");
        AV::ScriptUtils::addFunction(vm, setAutoOverlayEnabled, "setAutoOverlayEnabled", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getAutoOverlayEnabled, "getAutoOverlayEnabled", 1, ".");

        //Windows
        AV::ScriptUtils::addFunction(vm, begin, "begin", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginClosable, "beginClosable", -2, ".si");
        AV::ScriptUtils::addFunction(vm, end, "end", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginChild, "beginChild", -2, ".snnii");
        AV::ScriptUtils::addFunction(vm, endChild, "endChild", 1, ".");
        AV::ScriptUtils::addFunction(vm, setNextWindowPos, "setNextWindowPos", -3, ".nninn");
        AV::ScriptUtils::addFunction(vm, setNextWindowSize, "setNextWindowSize", -3, ".nni");
        AV::ScriptUtils::addFunction(vm, setNextWindowCollapsed, "setNextWindowCollapsed", -2, ".bi");
        AV::ScriptUtils::addFunction(vm, setNextWindowFocus, "setNextWindowFocus", 1, ".");
        AV::ScriptUtils::addFunction(vm, setNextWindowBgAlpha, "setNextWindowBgAlpha", 2, ".n");
        AV::ScriptUtils::addFunction(vm, getWindowPos, "getWindowPos", 1, ".");
        AV::ScriptUtils::addFunction(vm, getWindowSize, "getWindowSize", 1, ".");
        AV::ScriptUtils::addFunction(vm, getWindowWidth, "getWindowWidth", 1, ".");
        AV::ScriptUtils::addFunction(vm, getWindowHeight, "getWindowHeight", 1, ".");
        AV::ScriptUtils::addFunction(vm, getContentRegionAvail, "getContentRegionAvail", 1, ".");
        AV::ScriptUtils::addFunction(vm, isWindowHovered, "isWindowHovered", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isWindowFocused, "isWindowFocused", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isWindowCollapsed, "isWindowCollapsed", 1, ".");

        //Docking
        AV::ScriptUtils::addFunction(vm, dockSpace, "dockSpace", -2, ".s|inni");
        AV::ScriptUtils::addFunction(vm, dockSpaceOverViewport, "dockSpaceOverViewport", -1, ".s|i|oi");
        AV::ScriptUtils::addFunction(vm, setNextWindowDockId, "setNextWindowDockId", -2, ".s|ii");
        AV::ScriptUtils::addFunction(vm, getWindowDockId, "getWindowDockId", 1, ".");
        AV::ScriptUtils::addFunction(vm, isWindowDocked, "isWindowDocked", 1, ".");
        AV::ScriptUtils::addFunction(vm, setDockingEnabled, "setDockingEnabled", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getDockingEnabled, "getDockingEnabled", 1, ".");
        AV::ScriptUtils::addFunction(vm, setDockingNoSplit, "setDockingNoSplit", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getDockingNoSplit, "getDockingNoSplit", 1, ".");
        AV::ScriptUtils::addFunction(vm, setDockingWithShift, "setDockingWithShift", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getDockingWithShift, "getDockingWithShift", 1, ".");
        AV::ScriptUtils::addFunction(vm, setDockingAlwaysTabBar, "setDockingAlwaysTabBar", 2, ".b");
        AV::ScriptUtils::addFunction(vm, getDockingAlwaysTabBar, "getDockingAlwaysTabBar", 1, ".");

        //Dock builder
        AV::ScriptUtils::addFunction(vm, dockBuilderAddNode, "dockBuilderAddNode", -1, ".s|i|oi");
        AV::ScriptUtils::addFunction(vm, dockBuilderRemoveNode, "dockBuilderRemoveNode", 2, ".s|i");
        AV::ScriptUtils::addFunction(vm, dockBuilderSetNodeSize, "dockBuilderSetNodeSize", 4, ".s|inn");
        AV::ScriptUtils::addFunction(vm, dockBuilderSplitNode, "dockBuilderSplitNode", 4, ".s|iin");
        AV::ScriptUtils::addFunction(vm, dockBuilderDockWindow, "dockBuilderDockWindow", 3, ".ss|i");
        AV::ScriptUtils::addFunction(vm, dockBuilderFinish, "dockBuilderFinish", 2, ".s|i");

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
        AV::ScriptUtils::addFunction(vm, bullet, "bullet", 1, ".");

        //Images
        AV::ScriptUtils::addFunction(vm, image, "image", -4, ".unnnnnn");
        AV::ScriptUtils::addFunction(vm, imageButton, "imageButton", -5, ".sunnnnnnuu");

        //Value widgets
        AV::ScriptUtils::addFunction(vm, sliderFloat, "sliderFloat", -5, ".snnns|oi");
        AV::ScriptUtils::addFunction(vm, sliderInt, "sliderInt", -5, ".siiis|oi");
        AV::ScriptUtils::addFunction(vm, sliderFloatN, "sliderFloatN", -5, ".sanns|oi");
        AV::ScriptUtils::addFunction(vm, dragFloat, "dragFloat", -3, ".snnnns|oi");
        AV::ScriptUtils::addFunction(vm, dragInt, "dragInt", -3, ".sinnns|oi");
        AV::ScriptUtils::addFunction(vm, inputFloat, "inputFloat", -3, ".snnns|oi");
        AV::ScriptUtils::addFunction(vm, inputInt, "inputInt", -3, ".siiii");
        AV::ScriptUtils::addFunction(vm, dragFloat2, "dragFloat2", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, dragFloat3, "dragFloat3", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, dragFloat4, "dragFloat4", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, dragInt2, "dragInt2", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, dragInt3, "dragInt3", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, dragInt4, "dragInt4", -3, ".sannns|oi");
        AV::ScriptUtils::addFunction(vm, inputFloat2, "inputFloat2", -3, ".sas|oi");
        AV::ScriptUtils::addFunction(vm, inputFloat3, "inputFloat3", -3, ".sas|oi");
        AV::ScriptUtils::addFunction(vm, inputFloat4, "inputFloat4", -3, ".sas|oi");
        AV::ScriptUtils::addFunction(vm, inputInt2, "inputInt2", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, inputInt3, "inputInt3", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, inputInt4, "inputInt4", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, inputText, "inputText", -3, ".ssi");
        AV::ScriptUtils::addFunction(vm, inputTextMultiline, "inputTextMultiline", -3, ".ssnni");
        AV::ScriptUtils::addFunction(vm, colorEdit3, "colorEdit3", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, colorEdit4, "colorEdit4", -3, ".sai");
        AV::ScriptUtils::addFunction(vm, combo, "combo", -4, ".siai");
        AV::ScriptUtils::addFunction(vm, listBox, "listBox", -4, ".siai");

        //Trees
        AV::ScriptUtils::addFunction(vm, treeNode, "treeNode", 2, ".s");
        AV::ScriptUtils::addFunction(vm, treeNodeEx, "treeNodeEx", -2, ".si");
        AV::ScriptUtils::addFunction(vm, treePop, "treePop", 1, ".");
        AV::ScriptUtils::addFunction(vm, collapsingHeader, "collapsingHeader", -2, ".si");
        AV::ScriptUtils::addFunction(vm, setNextItemOpen, "setNextItemOpen", -2, ".bi");

        //Layout
        AV::ScriptUtils::addFunction(vm, sameLine, "sameLine", -1, ".nn");
        AV::ScriptUtils::addFunction(vm, newLine, "newLine", 1, ".");
        AV::ScriptUtils::addFunction(vm, spacing, "spacing", 1, ".");
        AV::ScriptUtils::addFunction(vm, separator, "separator", 1, ".");
        AV::ScriptUtils::addFunction(vm, dummy, "dummy", 3, ".nn");
        AV::ScriptUtils::addFunction(vm, indent, "indent", -1, ".n");
        AV::ScriptUtils::addFunction(vm, unindent, "unindent", -1, ".n");
        AV::ScriptUtils::addFunction(vm, beginGroup, "beginGroup", 1, ".");
        AV::ScriptUtils::addFunction(vm, endGroup, "endGroup", 1, ".");
        AV::ScriptUtils::addFunction(vm, alignTextToFramePadding, "alignTextToFramePadding", 1, ".");
        AV::ScriptUtils::addFunction(vm, setNextItemWidth, "setNextItemWidth", 2, ".n");
        AV::ScriptUtils::addFunction(vm, pushItemWidth, "pushItemWidth", 2, ".n");
        AV::ScriptUtils::addFunction(vm, popItemWidth, "popItemWidth", 1, ".");
        AV::ScriptUtils::addFunction(vm, getCursorPosX, "getCursorPosX", 1, ".");
        AV::ScriptUtils::addFunction(vm, getCursorPosY, "getCursorPosY", 1, ".");
        AV::ScriptUtils::addFunction(vm, setCursorPos, "setCursorPos", 3, ".nn");
        AV::ScriptUtils::addFunction(vm, getCursorScreenPos, "getCursorScreenPos", 1, ".");
        AV::ScriptUtils::addFunction(vm, calcTextSize, "calcTextSize", -2, ".sn");
        AV::ScriptUtils::addFunction(vm, getFrameHeight, "getFrameHeight", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginDisabled, "beginDisabled", -1, ".b");
        AV::ScriptUtils::addFunction(vm, endDisabled, "endDisabled", 1, ".");

        //Menus, tabs
        AV::ScriptUtils::addFunction(vm, beginMenuBar, "beginMenuBar", 1, ".");
        AV::ScriptUtils::addFunction(vm, endMenuBar, "endMenuBar", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginMainMenuBar, "beginMainMenuBar", 1, ".");
        AV::ScriptUtils::addFunction(vm, endMainMenuBar, "endMainMenuBar", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginMenu, "beginMenu", -2, ".sb");
        AV::ScriptUtils::addFunction(vm, endMenu, "endMenu", 1, ".");
        AV::ScriptUtils::addFunction(vm, menuItem, "menuItem", -2, ".ss|obb");
        AV::ScriptUtils::addFunction(vm, beginTabBar, "beginTabBar", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endTabBar, "endTabBar", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginTabItem, "beginTabItem", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endTabItem, "endTabItem", 1, ".");

        //Tables
        AV::ScriptUtils::addFunction(vm, beginTable, "beginTable", -3, ".siinn");
        AV::ScriptUtils::addFunction(vm, endTable, "endTable", 1, ".");
        AV::ScriptUtils::addFunction(vm, tableNextRow, "tableNextRow", -1, ".in");
        AV::ScriptUtils::addFunction(vm, tableNextColumn, "tableNextColumn", 1, ".");
        AV::ScriptUtils::addFunction(vm, tableSetColumnIndex, "tableSetColumnIndex", 2, ".i");
        AV::ScriptUtils::addFunction(vm, tableSetupColumn, "tableSetupColumn", -2, ".sin");
        AV::ScriptUtils::addFunction(vm, tableHeadersRow, "tableHeadersRow", 1, ".");

        //Popups, tooltips
        AV::ScriptUtils::addFunction(vm, openPopup, "openPopup", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginPopup, "beginPopup", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginPopupModal, "beginPopupModal", -2, ".si");
        AV::ScriptUtils::addFunction(vm, endPopup, "endPopup", 1, ".");
        AV::ScriptUtils::addFunction(vm, closeCurrentPopup, "closeCurrentPopup", 1, ".");
        AV::ScriptUtils::addFunction(vm, isPopupOpen, "isPopupOpen", -2, ".si");
        AV::ScriptUtils::addFunction(vm, beginTooltip, "beginTooltip", 1, ".");
        AV::ScriptUtils::addFunction(vm, beginItemTooltip, "beginItemTooltip", 1, ".");
        AV::ScriptUtils::addFunction(vm, endTooltip, "endTooltip", 1, ".");
        AV::ScriptUtils::addFunction(vm, setTooltip, "setTooltip", 2, ".s");

        //Plots
        AV::ScriptUtils::addFunction(vm, plotLines, "plotLines", -3, ".sas|onnnn");
        AV::ScriptUtils::addFunction(vm, plotHistogram, "plotHistogram", -3, ".sas|onnnn");

        //Item queries
        AV::ScriptUtils::addFunction(vm, isItemHovered, "isItemHovered", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isItemActive, "isItemActive", 1, ".");
        AV::ScriptUtils::addFunction(vm, isItemClicked, "isItemClicked", -1, ".i");
        AV::ScriptUtils::addFunction(vm, isItemEdited, "isItemEdited", 1, ".");
        AV::ScriptUtils::addFunction(vm, isItemActivated, "isItemActivated", 1, ".");
        AV::ScriptUtils::addFunction(vm, isItemDeactivated, "isItemDeactivated", 1, ".");
        AV::ScriptUtils::addFunction(vm, isItemDeactivatedAfterEdit, "isItemDeactivatedAfterEdit", 1, ".");
        AV::ScriptUtils::addFunction(vm, isAnyItemActive, "isAnyItemActive", 1, ".");
        AV::ScriptUtils::addFunction(vm, isAnyItemHovered, "isAnyItemHovered", 1, ".");
        AV::ScriptUtils::addFunction(vm, setItemDefaultFocus, "setItemDefaultFocus", 1, ".");
        AV::ScriptUtils::addFunction(vm, setKeyboardFocusHere, "setKeyboardFocusHere", -1, ".i");

        //ID, style
        AV::ScriptUtils::addFunction(vm, pushId, "pushId", 2, ".s|i");
        AV::ScriptUtils::addFunction(vm, popId, "popId", 1, ".");
        AV::ScriptUtils::addFunction(vm, getId, "getId", 2, ".s");
        AV::ScriptUtils::addFunction(vm, pushStyleColor, "pushStyleColor", 6, ".innnn");
        AV::ScriptUtils::addFunction(vm, popStyleColor, "popStyleColor", -1, ".i");
        AV::ScriptUtils::addFunction(vm, pushStyleVar, "pushStyleVar", -3, ".inn");
        AV::ScriptUtils::addFunction(vm, popStyleVar, "popStyleVar", -1, ".i");

        _setupConstants(vm);
    }
}
