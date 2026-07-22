# avImguiPlugin

A prebuilt [avEngine](https://github.com/OtherMythos/avEngine) plugin that exposes
[Dear ImGui](https://github.com/ocornut/imgui) to Squirrel as the `_imgui` namespace,
for debug tools and overlays.

This folder is the whole distribution. Drop it into a project's `plugins/`
directory, so it sits at `plugins/avImguiPlugin/`.

## Contents

| | |
|---|---|
| `bin/lib*.so`, `bin/*.dll` | Desktop plugins, loaded at runtime. The engine picks the one matching the platform, architecture and build type. |
| `bin/lib*_ios_static_*.a` | Static library for platforms that can't load shared libraries. |
| `plugin.cmake` | Links that static library into an engine build. |
| `include/AvImguiPlugin.h` | The plugin class, for a project's `StaticPlugins.h`. |

## Use it

Point the project's `avSetup.cfg` at `bin/`:

```json
"Plugins": [
    { "name": "AvImguiPlugin", "path": "res://plugins/avImguiPlugin/bin" }
]
```

Then build a gui from any `update` callback — the frame is begun and rendered for you:

```squirrel
function update(){
    _imgui.begin("Debug");
    _imgui.text("Hello");
    _imgui.end();
}
```

The plugin prints its version on startup, so check stdout if something looks off.

## Static platforms (iOS)

Two extra lines. In the project's `CMakeLists.txt` (the one the engine loads via
`AV_PROJECT_DIR`):

```cmake
list(APPEND StaticPluginIncludes ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/plugins/avImguiPlugin/plugin.cmake)
```

and in the project's `StaticPlugins.h`:

```cpp
#include "AvImguiPlugin.h"

void registerStaticPlugins(){
    REGISTER_PLUGIN("AvImguiPlugin", AVImgui::AvImguiPlugin)
}
```

Then build the engine with `-DUSE_STATIC_PLUGINS=True -DAV_PROJECT_DIR=<project>`.

## Notes

- imgui writes an `imgui.ini` (window positions) into the working directory — worth
  adding to a project's `.gitignore`.
- Full API reference and source: https://github.com/OtherMythos/avEngineIMGUI
