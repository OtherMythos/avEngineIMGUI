# Examples

Two small avEngine projects, each a complete `avSetup.cfg` plus one script.

| | |
|---|---|
| [demo/](demo) | A debug tools overlay: menus, value widgets, a table, a frame time plot. A tour of the general api. |
| [docking/](docking) | The docking api: a full screen dockspace, windows docked into it from script, a nested dockspace, and the docking options. |

Both load the plugin from `../../plugins/avImguiPlugin`, which is where the
distribution is extracted. Build the plugin first (see the [main README](../README.md#building))
and stage it there, or drop in a downloaded distribution:

```bash
cp native/build/plugin/libAvImguiPlugin.so \
   plugins/avImguiPlugin/bin/libAvImguiPlugin_Debug-macos-arm64.so
```

Then run either project by pointing the engine at its setup file:

```bash
/path/to/avEngine/build/Debug/av.app/Contents/MacOS/av examples/docking/avSetup.cfg
```

The plugin prints its version as it loads, so stdout says which imgui is
actually running:

```
AvImguiPlugin 0.1.0 unstable (6d8c410) with Dear ImGui 1.92.8 (docking)
```

## What to try in the docking example

- Drag a window by its title bar or tab onto another to merge them into a tab
  bar, or toward an edge to split the space.
- Drag a tab out to undock it. It stays inside the engine window: multi-viewport
  is not available, see the [main README](../README.md#docking).
- The middle of the screen is a transparent central node
  (`DockNodeFlags_PassthruCentralNode`), so a game would still be visible and
  clickable through it.
- *Windows → Workspace* holds a dockspace of its own, with the Inspector docked
  inside it. Undock the Workspace window and its contents travel with it.
- *Always floating* uses `WindowFlags_NoDocking` and refuses to dock anywhere.
- The *Docking* menu toggles the options: no split, dock only while shift is
  held, and a tab bar on every floating window.

Dock layouts are not remembered between runs — the plugin disables imgui's
settings persistence, so there is no `imgui.ini`. That is why the example places
its windows with `setNextWindowDockId` and `Cond_FirstUseEver`: docked where they
belong on first run, free to be rearranged afterwards.
