//An example debug tools overlay built with the _imgui namespace.
//Run with the avSetup.cfg in this directory (with plugin binaries in plugins/).

//Persistent gui state. Immediate mode guis keep state in your code.
local state = {
    showDemo = false,
    wireframe = false,
    speed = 1.0,
    playerName = "player",
    tint = [1.0, 0.5, 0.2],
    mode = 0,
    frameTimes = array(60, 0.0),
    frameIdx = 0,
    lastTime = 0.0
};

function start(){
    print("imgui version: " + _imgui.getVersion());
}

function update(){
    //The engine can run several fixed updates per rendered frame; only build
    //the gui once per rendered frame.
    if(!_imgui.isFirstUpdateOfFrame()) return;

    //Record a rolling frame time history for the plot.
    local now = _imgui.getTime();
    state.frameTimes[state.frameIdx] = (now - state.lastTime) * 1000.0;
    state.lastTime = now;
    state.frameIdx = (state.frameIdx + 1) % state.frameTimes.len();

    _imgui.setNextWindowPos(20, 20, _imgui.Cond_FirstUseEver);
    _imgui.setNextWindowSize(320, 400, _imgui.Cond_FirstUseEver);

    if(_imgui.begin("Debug tools", _imgui.WindowFlags_MenuBar)){

        if(_imgui.beginMenuBar()){
            if(_imgui.beginMenu("Windows")){
                if(_imgui.menuItem("ImGui demo", null, state.showDemo)){
                    state.showDemo = !state.showDemo;
                }
                _imgui.endMenu();
            }
            _imgui.endMenuBar();
        }

        _imgui.separatorText("Rendering");
        state.wireframe = _imgui.checkbox("Wireframe", state.wireframe);
        if(_imgui.colorEdit3("Tint", state.tint)){
            //Arrays are modified in place; react to the change here.
            //_myGame.setTint(state.tint[0], state.tint[1], state.tint[2]);
        }

        _imgui.separatorText("Gameplay");
        state.speed = _imgui.sliderFloat("Speed", state.speed, 0.0, 10.0);
        state.playerName = _imgui.inputText("Name", state.playerName);
        state.mode = _imgui.combo("Mode", state.mode, ["Explore", "Combat", "Cutscene"]);

        if(_imgui.button("Reset")){
            state.speed = 1.0;
            state.mode = 0;
        }
        _imgui.sameLine();
        _imgui.textDisabled("speed " + state.speed);

        if(_imgui.collapsingHeader("Performance")){
            _imgui.plotLines("frame", state.frameTimes);
            _imgui.text("imgui frames: " + _imgui.getFrameCount());
        }

        if(_imgui.beginTable("entities", 2, _imgui.TableFlags_Borders | _imgui.TableFlags_RowBg)){
            _imgui.tableSetupColumn("Entity");
            _imgui.tableSetupColumn("Health");
            _imgui.tableHeadersRow();
            foreach(idx, val in ["goblin", "orc", "dragon"]){
                _imgui.tableNextRow();
                _imgui.tableNextColumn();
                _imgui.text(val);
                _imgui.tableNextColumn();
                _imgui.text((100 - idx * 25).tostring());
            }
            _imgui.endTable();
        }
    }
    _imgui.end();

    if(state.showDemo){
        _imgui.showDemoWindow();
    }

    //Game input handling should typically skip while the gui uses the mouse.
    if(!_imgui.wantCaptureMouse()){
        //handleGameMouseInput();
    }
}
