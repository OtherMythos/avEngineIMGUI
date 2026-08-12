//An example of the docking api, from the docking branch of imgui.
//Run with the avSetup.cfg in this directory (with plugin binaries in plugins/).
//
//Docking lets tool windows be merged into tab bars and snapped into regions
//instead of floating loose over the game. It needs nothing to be set up: this
//example is about the api for placing windows from script, and for making a
//region they can be docked into.
//
//Two rules shape the order of everything below, because breaking either raises
//an imgui assertion which stops the engine rather than a recoverable error:
//
//  1. A dockspace must be submitted every frame it is in use. One that stops
//     being submitted has its node dropped and its windows undocked.
//  2. A dockspace must be submitted before the windows which dock into it. A
//     window submitted first is undocked again.
//
//So the frame goes: dockspace, then menu bar, then the windows.

local state = {
    //The id of the main dockspace, refreshed every frame.
    dockId = 0,
    //The id of the nested dockspace inside the Workspace window.
    workspaceDockId = 0,

    showEntities = true,
    showLog = true,
    showWorkspace = true,
    showFloating = true,

    //The docking options, mirrored here because imgui owns the real values and
    //the menu needs something to show a tick against.
    dockingEnabled = true,
    noSplit = false,
    withShift = false,
    alwaysTabBar = false,

    entities = ["goblin", "orc", "dragon", "slime"],
    selected = 0,
    log = ["engine started", "plugin loaded", "docking example running"]
};

local function yesNo(value){
    return value ? "yes" : "no";
}

function start(){
    print("imgui version: " + _imgui.getVersion());
    //Docking is on by default; the plugin sets it when it creates the context.
    print("docking enabled: " + yesNo(_imgui.getDockingEnabled()));
}

//The tool windows. Each is docked into the main dockspace the first time it is
//seen, and left alone after that so dragging it somewhere else sticks.
function entitiesWindow(){
    _imgui.setNextWindowDockId(state.dockId, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Entities")){
        foreach(idx, name in state.entities){
            if(_imgui.selectable(name, idx == state.selected)){
                state.selected = idx;
            }
        }
    }
    //end is always called, whether or not begin returned true.
    _imgui.end();
}

function logWindow(){
    _imgui.setNextWindowDockId(state.dockId, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Log")){
        foreach(line in state.log){
            _imgui.textWrapped(line);
        }
    }
    _imgui.end();
}

//A dockspace does not have to fill the screen: one inside a window gives that
//window its own docking area, independent of the main one.
function workspaceWindow(){
    _imgui.setNextWindowDockId(state.dockId, _imgui.Cond_FirstUseEver);
    _imgui.begin("Workspace");
    //Called whether or not the window is visible. When it is hidden imgui adds
    //DockNodeFlags_KeepAliveOnly itself, which is what stops the windows docked
    //inside from being undocked while the tab is not selected.
    state.workspaceDockId = _imgui.dockSpace("workspace");
    _imgui.end();

    //Docked into the nested dockspace rather than the main one. Submitted after
    //the Workspace window, per rule 2 above.
    _imgui.setNextWindowDockId(state.workspaceDockId, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Inspector")){
        _imgui.text("Selected: " + state.entities[state.selected]);
        _imgui.separator();
        _imgui.text("A window docked into the Workspace");
        _imgui.text("window's own dockspace.");
    }
    _imgui.end();
}

//WindowFlags_NoDocking opts a window out entirely: it cannot be docked, by
//dragging or from script. Useful for a HUD or a transient overlay.
function floatingWindow(){
    _imgui.setNextWindowPos(40, 60, _imgui.Cond_FirstUseEver);
    _imgui.setNextWindowSize(260, 120, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Always floating", _imgui.WindowFlags_NoDocking)){
        _imgui.textWrapped("WindowFlags_NoDocking, so this one stays loose no matter where it is dragged.");
        _imgui.text("Docked: " + yesNo(_imgui.isWindowDocked()));
    }
    _imgui.end();
}

//Shows what the docking queries report for the window they are called in.
function stateWindow(){
    _imgui.setNextWindowDockId(state.dockId, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Docking state")){
        //These describe the window they are called inside, so they report this
        //window rather than anything global.
        _imgui.text("isWindowDocked: " + yesNo(_imgui.isWindowDocked()));
        _imgui.text("getWindowDockId: " + _imgui.getWindowDockId());
        _imgui.separator();
        _imgui.text("main dockspace: " + state.dockId);
        _imgui.text("workspace dockspace: " + state.workspaceDockId);
        _imgui.separator();
        //getId is the hash imgui gives a string, and is what dockSpace does to
        //a string id internally. It is relative to the current window, which is
        //why the dockspace calls return their id rather than expecting scripts
        //to work it out again elsewhere.
        _imgui.text("getId(\"workspace\") here: " + _imgui.getId("workspace"));
        _imgui.textDisabled("Not the same as the id above: a different window.");
    }
    _imgui.end();
}

function menuBar(){
    if(!_imgui.beginMainMenuBar()) return;

    if(_imgui.beginMenu("Windows")){
        if(_imgui.menuItem("Entities", null, state.showEntities)) state.showEntities = !state.showEntities;
        if(_imgui.menuItem("Log", null, state.showLog)) state.showLog = !state.showLog;
        if(_imgui.menuItem("Workspace", null, state.showWorkspace)) state.showWorkspace = !state.showWorkspace;
        if(_imgui.menuItem("Always floating", null, state.showFloating)) state.showFloating = !state.showFloating;
        _imgui.endMenu();
    }

    if(_imgui.beginMenu("Docking")){
        if(_imgui.menuItem("Enabled", null, state.dockingEnabled)){
            state.dockingEnabled = !state.dockingEnabled;
            _imgui.setDockingEnabled(state.dockingEnabled);
        }
        _imgui.separator();
        //Merge into tab bars only, never split a node in two.
        if(_imgui.menuItem("No split", null, state.noSplit)){
            state.noSplit = !state.noSplit;
            _imgui.setDockingNoSplit(state.noSplit);
        }
        //Require shift to be held, so windows are not docked by accident.
        if(_imgui.menuItem("Dock with shift", null, state.withShift)){
            state.withShift = !state.withShift;
            _imgui.setDockingWithShift(state.withShift);
        }
        //Give every floating window a tab bar, so anything can be docked into.
        if(_imgui.menuItem("Always tab bar", null, state.alwaysTabBar)){
            state.alwaysTabBar = !state.alwaysTabBar;
            _imgui.setDockingAlwaysTabBar(state.alwaysTabBar);
        }
        _imgui.endMenu();
    }

    _imgui.endMainMenuBar();
}

function update(){
    //The engine can run several fixed updates per rendered frame; only build
    //the gui once per rendered frame.
    if(!_imgui.isFirstUpdateOfFrame()) return;

    //A dockspace covering the whole display, submitted first so every window
    //below can dock into it. PassthruCentralNode leaves the empty middle
    //transparent and lets clicks through it, so the game is still visible and
    //usable underneath; without it the middle is filled with Col_DockingEmptyBg.
    //
    //Guarded because the dockspace calls are an error while docking is off,
    //rather than quietly drawing nothing — and the Docking menu can turn it off.
    if(_imgui.getDockingEnabled()){
        state.dockId = _imgui.dockSpaceOverViewport(null, _imgui.DockNodeFlags_PassthruCentralNode);
    }else{
        state.dockId = 0;
    }

    menuBar();

    //Docked windows come after the dockspace they dock into.
    if(state.showEntities) entitiesWindow();
    if(state.showLog) logWindow();
    if(state.showWorkspace) workspaceWindow();
    stateWindow();
    if(state.showFloating) floatingWindow();
}
