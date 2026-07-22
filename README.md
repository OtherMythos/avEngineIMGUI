# avEngineIMGUI

An [avEngine](https://github.com/OtherMythos/avEngine) plugin which exposes [Dear ImGui](https://github.com/ocornut/imgui) to Squirrel scripts.

Dear ImGui is an immediate mode gui, which makes it a great fit for debug tooling: scripts re-build the gui every frame with plain function calls, with no widget objects to create or destroy. This plugin bakes the imgui library (1.92.8) and an Ogre-next renderer into a single loadable plugin, keeping imgui an optional dependency of the engine. When the plugin loads it defines a namespace called `_imgui` containing the api.

```squirrel
function update(){
    _imgui.begin("Debug");
    _imgui.text("Entities: " + count);
    if(_imgui.button("Spawn")){
        spawnEntity();
    }
    mSpeed = _imgui.sliderFloat("Speed", mSpeed, 0.0, 10.0);
    _imgui.end();
}
```

## Quick start

1. Download the plugin binaries from the actions distribution artifact (or build them, see below) and place them in a `plugins` directory inside your project. The engine picks the correct binary by platform, architecture and build type from the file name, so a single directory can hold every variant:

```
myProject/
  avSetup.cfg
  plugins/
    libAvImguiPlugin_Debug-macos-arm64.so
    libAvImguiPlugin_Release-macos-arm64.so
    libAvImguiPlugin_Debug-linux-x86_64.so
    AvImguiPlugin_Release-windows-x86_64.dll
    ...
```

2. Register the plugin in `avSetup.cfg`:

```json
"Plugins": [
    {
        "name": "AvImguiPlugin",
        "path": "res://plugins"
    }
]
```

`path` may also be an array of candidate locations (directories or direct file paths), tried in order. This is convenient during development to prefer a locally built binary:

```json
"Plugins": [
    {
        "name": "AvImguiPlugin",
        "path": [
            "res://native/build/plugin/libAvImguiPlugin.so",
            "res://plugins"
        ]
    }
]
```

3. Call `_imgui` functions from an `update` callback. There is no frame lifecycle to manage — the first `_imgui` call of each frame begins the imgui frame, and the compositor renders it after the scene:

```squirrel
function update(){
    _imgui.showDemoWindow();
}
```

## The frame model

- The engine has no per-frame plugin hook, so the imgui frame is begun *lazily*: the first `_imgui` call in an engine frame calls `ImGui::NewFrame()` internally. Rendering happens later in the frame inside a custom compositor pass. If no `_imgui` calls are made in a frame, nothing is rendered.
- The engine runs script updates on a *fixed timestep*, decoupled from the framerate:
  - When the framerate is higher than the fixed update rate, some rendered frames receive no script update. The plugin re-presents the previous frame's gui for those frames, so the gui does not flicker.
  - When the framerate is lower than the fixed update rate, a single rendered frame can receive several update calls, and widgets submitted twice in one imgui frame appear twice. Scripts that care can guard with `isFirstUpdateOfFrame`:

```squirrel
function update(){
    if(!_imgui.isFirstUpdateOfFrame()) return;
    //Build the gui...
}
```

## Rendering and the compositor

The plugin renders through a custom compositor pass with the custom id `imgui`. Ogre allows a single compositor pass provider, which the engine already uses for its own passes (`rect2d`, `colibri_gui`), so the plugin *wraps* the engine's provider and delegates every other custom pass id to it.

By default nothing needs configuring. The first frame which actually uses imgui creates an overlay workspace (`avImgui/Workspace`) that draws the gui over whatever has already been rendered. Because Ogre appends new workspaces, the overlay always executes after the workspaces that exist at that moment — so this works whether the project uses the engine's default compositor or builds its own workspaces from script during `start()`.

Two cases need attention:

- **Workspaces created after imgui is first used.** A workspace added later executes after the overlay and would draw over the gui. Recreate the overlay so it goes last again:

```squirrel
_imgui.destroyOverlayWorkspace();
_imgui.createOverlayWorkspace();
```

- **Projects rendering imgui through their own compositor** (using `pass custom imgui`, below) should turn the overlay off during setup, otherwise the gui is rendered twice:

```squirrel
function start(){
    _imgui.setAutoOverlayEnabled(false);
}
```

- **Custom compositor scripts**: `pass custom imgui` can be used directly inside a `.compositor` file, placed as the final pass of the target you want the gui drawn over. Note that compositor scripts parsed during engine startup are parsed *before* plugins load and cannot reference the pass — this only works for compositor files in resource groups initialised later from script (`_resources.initialiseAllResourceGroups()`), which is the usual pattern for projects with custom compositors.

```
compositor_node myNode{
    in 0 rt_renderwindow

    target rt_renderwindow{
        pass render_scene{ }
        pass custom imgui{ }
    }
}
```

## Input

The plugin observes the engine's SDL event stream directly (mouse position, buttons, wheel, keyboard, text input), so the full imgui interaction model works, including text fields. Two things to be aware of:

- Input is *observed*, not consumed: clicks over an imgui window still reach the engine's own input system. Game code which should ignore input while the gui is using it can check:

```squirrel
if(_imgui.wantCaptureMouse()){
    //imgui is using the mouse, skip game mouse handling.
}
```

- `wantCaptureKeyboard()` and `wantTextInput()` work the same way for the keyboard. While an imgui text field is active the plugin enables SDL text input itself; disabling it again is left to the engine's own gui focus handling.

---

# API reference

General conventions:

- Names are camelCase versions of the imgui api: `ImGui::SliderFloat` is `_imgui.sliderFloat`, `ImGui::BeginTabBar` is `_imgui.beginTabBar`.
- imgui writes widget values through pointers; squirrel has no reference primitives, so **value widgets take the current value and return the new value**:

```squirrel
mSpeed = _imgui.sliderFloat("Speed", mSpeed, 0.0, 10.0);
mEnabled = _imgui.checkbox("Enabled", mEnabled);
mName = _imgui.inputText("Name", mName);
mIndex = _imgui.combo("Mode", mIndex, ["First", "Second", "Third"]);
```

- **Array widgets mutate the passed array in place** (arrays are reference types in squirrel) and return whether the value changed:

```squirrel
local col = [1.0, 0.5, 0.2];
if(_imgui.colorEdit3("Tint", col)){
    applyTint(col[0], col[1], col[2]);
}
```

- To detect edits of scalar widgets use the item query functions: `_imgui.sliderFloat(...)` followed by `_imgui.isItemEdited()` or `_imgui.isItemDeactivatedAfterEdit()`.
- Positions and sizes are plain number pairs, and functions returning a pair return a two element array `[x, y]`.
- Flag and enum constants are integers in the namespace, named after the imgui enum without the `ImGui` prefix: `ImGuiWindowFlags_NoResize` is `_imgui.WindowFlags_NoResize`. Combine with `|`.
- Text is never interpreted as a format string, so any characters are safe to display.
- imgui pairing rules apply: `begin`/`end` must always be paired, while `beginChild`, `beginTabBar`, `beginMenu`, `beginPopup`, `beginTable` and friends follow imgui's rule that the matching `end*` is only called when the begin returned `true` (except `endChild`, which is always required).

### Windows

| Function | Notes |
|---|---|
| `begin(name, flags = 0)` → bool | Begin a window. Returns false when collapsed. Always pair with `end()`. |
| `end()` | |
| `beginChild(id, w = 0, h = 0, childFlags = 0, windowFlags = 0)` → bool | Always pair with `endChild()`. |
| `endChild()` | |
| `setNextWindowPos(x, y, cond = 0, pivotX = 0, pivotY = 0)` | e.g. `cond = _imgui.Cond_FirstUseEver` |
| `setNextWindowSize(w, h, cond = 0)` | |
| `setNextWindowCollapsed(collapsed, cond = 0)` | |
| `setNextWindowFocus()` | |
| `setNextWindowBgAlpha(alpha)` | |
| `getWindowPos()` → [x, y] | |
| `getWindowSize()` → [w, h] | |
| `getWindowWidth()` / `getWindowHeight()` → float | |
| `getContentRegionAvail()` → [w, h] | |
| `isWindowHovered(flags = 0)` / `isWindowFocused(flags = 0)` / `isWindowCollapsed()` → bool | |

### Text

| Function | Notes |
|---|---|
| `text(str)` | |
| `textColored(r, g, b, a, str)` | |
| `textDisabled(str)` / `textWrapped(str)` / `bulletText(str)` | |
| `labelText(label, str)` | Value aligned like other widgets. |
| `separatorText(str)` | Separator with a heading. |

### Main widgets

| Function | Notes |
|---|---|
| `button(label, w = 0, h = 0)` → bool | True when pressed. |
| `smallButton(label)` → bool | |
| `invisibleButton(id, w, h, flags = 0)` → bool | |
| `arrowButton(id, dir)` → bool | `dir` is a `Dir_` constant. |
| `checkbox(label, checked)` → bool | Returns the new state. |
| `radioButton(label, active)` → bool | True when pressed. |
| `selectable(label, selected = false, flags = 0, w = 0, h = 0)` → bool | True when clicked. |
| `progressBar(fraction, w = -remaining, h = 0, overlay = null)` | |
| `bullet()` | |

### Value widgets

| Function | Notes |
|---|---|
| `sliderFloat(label, v, min, max, format = "%.3f", flags = 0)` → float | |
| `sliderInt(label, v, min, max, format = "%d", flags = 0)` → int | |
| `sliderFloatN(label, array, min, max, format = "%.3f", flags = 0)` → bool | 1-4 element array, mutated in place. |
| `dragFloat(label, v, speed = 1, min = 0, max = 0, format = "%.3f", flags = 0)` → float | |
| `dragInt(label, v, speed = 1, min = 0, max = 0, format = "%d", flags = 0)` → int | |
| `inputFloat(label, v, step = 0, stepFast = 0, format = "%.3f", flags = 0)` → float | |
| `inputInt(label, v, step = 1, stepFast = 100, flags = 0)` → int | |
| `inputText(label, str, flags = 0)` → string | |
| `inputTextMultiline(label, str, w = 0, h = 0, flags = 0)` → string | |
| `colorEdit3(label, [r, g, b], flags = 0)` → bool | Array mutated in place. |
| `colorEdit4(label, [r, g, b, a], flags = 0)` → bool | |
| `combo(label, currentIndex, itemsArray, popupMaxHeightInItems = -1)` → int | Returns the new index. |
| `listBox(label, currentIndex, itemsArray, heightInItems = -1)` → int | |

### Trees and headers

| Function | Notes |
|---|---|
| `treeNode(label)` → bool | Pair with `treePop()` when true. |
| `treeNodeEx(label, flags = 0)` → bool | |
| `treePop()` | |
| `collapsingHeader(label, flags = 0)` → bool | No `treePop` needed. |
| `setNextItemOpen(open, cond = 0)` | |

### Layout

`sameLine(offsetX = 0, spacing = -1)`, `newLine()`, `spacing()`, `separator()`, `dummy(w, h)`, `indent(w = 0)`, `unindent(w = 0)`, `beginGroup()`, `endGroup()`, `alignTextToFramePadding()`, `setNextItemWidth(w)`, `pushItemWidth(w)`, `popItemWidth()`, `getCursorPosX()`, `getCursorPosY()`, `setCursorPos(x, y)`, `getCursorScreenPos()` → [x, y], `calcTextSize(str, wrapWidth = -1)` → [w, h], `getFrameHeight()`, `beginDisabled(disabled = true)`, `endDisabled()`

### Menus and tabs

| Function | Notes |
|---|---|
| `beginMenuBar()` → bool | Window needs `WindowFlags_MenuBar`. |
| `endMenuBar()` | |
| `beginMainMenuBar()` → bool / `endMainMenuBar()` | Menu bar at the top of the screen. |
| `beginMenu(label, enabled = true)` → bool / `endMenu()` | |
| `menuItem(label, shortcut = null, selected = false, enabled = true)` → bool | True when activated. |
| `beginTabBar(id, flags = 0)` → bool / `endTabBar()` | |
| `beginTabItem(label, flags = 0)` → bool / `endTabItem()` | |

### Tables

| Function | Notes |
|---|---|
| `beginTable(id, columns, flags = 0, w = 0, h = 0)` → bool | |
| `endTable()` | Only when `beginTable` returned true. |
| `tableNextRow(rowFlags = 0, minRowHeight = 0)` | |
| `tableNextColumn()` → bool | |
| `tableSetColumnIndex(index)` → bool | |
| `tableSetupColumn(label, flags = 0, initWidthOrWeight = 0)` | |
| `tableHeadersRow()` | |

```squirrel
if(_imgui.beginTable("entities", 2, _imgui.TableFlags_Borders | _imgui.TableFlags_RowBg)){
    _imgui.tableSetupColumn("Name");
    _imgui.tableSetupColumn("Position");
    _imgui.tableHeadersRow();
    foreach(e in entities){
        _imgui.tableNextRow();
        _imgui.tableNextColumn();
        _imgui.text(e.name);
        _imgui.tableNextColumn();
        _imgui.text(e.pos.tostring());
    }
    _imgui.endTable();
}
```

### Popups and tooltips

| Function | Notes |
|---|---|
| `openPopup(id, flags = 0)` | |
| `beginPopup(id, flags = 0)` → bool / `endPopup()` | `endPopup` only when true. |
| `beginPopupModal(name, flags = 0)` → bool | |
| `closeCurrentPopup()` | |
| `isPopupOpen(id, flags = 0)` → bool | |
| `beginTooltip()` → bool / `endTooltip()` | |
| `beginItemTooltip()` → bool | Tooltip when the previous item is hovered. |
| `setTooltip(str)` | |

### Plots

| Function | Notes |
|---|---|
| `plotLines(label, valuesArray, overlay = null, scaleMin = auto, scaleMax = auto, w = 0, h = 0)` | |
| `plotHistogram(label, valuesArray, overlay = null, scaleMin = auto, scaleMax = auto, w = 0, h = 0)` | |

### Item queries

`isItemHovered(flags = 0)`, `isItemActive()`, `isItemClicked(button = 0)`, `isItemEdited()`, `isItemActivated()`, `isItemDeactivated()`, `isItemDeactivatedAfterEdit()`, `isAnyItemActive()`, `isAnyItemHovered()`, `setItemDefaultFocus()`, `setKeyboardFocusHere(offset = 0)`

### ID stack and style

| Function | Notes |
|---|---|
| `pushId(stringOrInt)` / `popId()` | Disambiguate identical labels, e.g. in loops. |
| `pushStyleColor(colConstant, r, g, b, a)` / `popStyleColor(count = 1)` | |
| `pushStyleVar(varConstant, value)` or `pushStyleVar(varConstant, x, y)` / `popStyleVar(count = 1)` | |

### Misc

| Function | Notes |
|---|---|
| `showDemoWindow()` | The full imgui demo — the best reference for what is possible. |
| `showMetricsWindow()` | imgui internals/metrics. |
| `getVersion()` → string | The imgui version, e.g. `"1.92.8"`. |
| `getTime()` → float / `getFrameCount()` → int | |
| `getDisplaySize()` → [w, h] | |
| `isFirstUpdateOfFrame()` → bool | See the frame model section. |
| `wantCaptureMouse()` / `wantCaptureKeyboard()` / `wantTextInput()` → bool | |
| `setRenderingEnabled(bool)` | Quickly hide/show the gui without changing script logic. |
| `createOverlayWorkspace()` → bool | Usually automatic. True if it was created, false if it already existed. |
| `destroyOverlayWorkspace()` → bool | Also disables automatic re-creation. |
| `setAutoOverlayEnabled(bool)` / `getAutoOverlayEnabled()` → bool | Turn off when rendering imgui through your own compositor pass. |

### Constants

Integer constants mirror the imgui enums, without the `ImGui` prefix:

`WindowFlags_*`, `ChildFlags_*`, `Cond_*`, `Dir_*`, `Col_*`, `StyleVar_*`, `TreeNodeFlags_*`, `InputTextFlags_*`, `SelectableFlags_*`, `ComboFlags_*`, `TableFlags_*`, `TableColumnFlags_*`, `TableRowFlags_*`, `TabBarFlags_*`, `TabItemFlags_*`, `HoveredFlags_*`, `FocusedFlags_*`, `PopupFlags_*`, `SliderFlags_*`, `ColorEditFlags_*`, `MouseButton_*`

See [native/plugin/src/Scripting/ImguiConstants.cpp](native/plugin/src/Scripting/ImguiConstants.cpp) for the full list.

---

# Building

The plugin builds out-of-tree against the engine source and the standard [avBuild](https://github.com/OtherMythos/avBuild) prebuilt dependencies:

```bash
mkdir -p native/build && cd native/build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DENGINE_SOURCE_PATH=/path/to/avEngine \
      -DAV_LIBS_DIR=/path/to/avBuilt/Debug \
      ..
cmake --build .
```

This produces `plugin/libAvImguiPlugin.so` (`AvImguiPlugin.dll` on Windows) and `plugin/libAvImguiPlugin_static.a`. On macOS and Linux the module leaves engine and Ogre symbols undefined, resolving them from the host executable at load time; on Windows it links against `avCore.lib`, Ogre, Squirrel and SDL2 from the dependency directory (the engine's import library is searched at `${ENGINE_SOURCE_PATH}/build/${CMAKE_BUILD_TYPE}`).

Dear ImGui itself is vendored in [native/imgui](native/imgui) and compiled into the plugin — the engine and project never need an imgui dependency.

## Static builds (iOS)

Platforms without dynamic loading link the plugin statically into the engine. Building with the iOS toolchain (`-DPLATFORM=OS64`) produces only `AvImguiPlugin_static`:

```bash
cmake -DAV_LIBS_DIR=/path/to/avBuiltIos/Debug -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_TOOLCHAIN_FILE=/path/to/avEngine/CMake/iosbuild.toolchain.cmake \
      -GXcode -DPLATFORM=OS64 -DENGINE_SOURCE_PATH=/path/to/avEngine ..
```

The engine build is then pointed at this repo (or an expanded distribution artifact, which has the same layout):

```bash
cmake -DUSE_STATIC_PLUGINS=True -DAV_PROJECT_DIR=/path/to/avEngineIMGUI ... # engine build
```

The engine compiles [StaticPlugins.h](StaticPlugins.h), which registers the plugin with `REGISTER_PLUGIN`, and the root [CMakeLists.txt](CMakeLists.txt) locates the prebuilt static library and exports it to the engine build. A game project with its own static plugins instead adds the equivalent `REGISTER_PLUGIN("AvImguiPlugin", AVImgui::AvImguiPlugin)` line to its own `StaticPlugins.h` and appends the library to its `StaticPluginLibraries`.

## CI

The [build workflow](.github/workflows/build.yml) builds Debug and Release binaries for Linux x86_64, macOS arm64, Windows x86_64 and an iOS static library, using the prebuilt dependency artifacts from `OtherMythos/avBuild` and the engine source from `OtherMythos/avEngine`. The `distribution` job bundles every binary together with the static integration files into a single `avImguiPlugin-distribution` artifact, named so the engine's runtime plugin search picks the right file per platform.

# Architecture notes

- `native/plugin/src/ImguiOgre/` is a baked-in copy of the renderer from [ogre-next-imgui](https://github.com/edherbert/ogre-next-imgui), ported to imgui 1.92 (texture id api, event based input) and re-architected to render from a compositor pass instead of a frame listener. The imgui shaders for Metal, Vulkan, D3D11 and OpenGL are embedded as source strings, so no resource files are required.
- `native/plugin/src/Compositor/` implements the `imgui` custom pass and the delegating pass provider.
- `native/plugin/src/Input/ImguiInput.cpp` installs an `SDL_AddEventWatch` to observe the event stream the engine pumps, translating events into imgui's io queue. This is what makes text input work without engine changes.
- `native/plugin/src/Scripting/` contains the `_imgui` namespace bindings and constants.

# License

MIT, see [LICENSE](LICENSE). Dear ImGui is vendored under its own MIT license in [native/imgui/LICENSE.txt](native/imgui/LICENSE.txt).
