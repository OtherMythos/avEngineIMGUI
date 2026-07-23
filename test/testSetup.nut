//Test harness for the _imgui api tests.
//
//Modelled on ProceduralExplorationGame's test/testSetup.nut, but much smaller:
//these tests only need a live engine with the plugin loaded, not a game.
//
//A test is a name, a description and a closure. Each runs on its own frame, so
//every test gets a fresh imgui frame (isFirstUpdateOfFrame is true when it
//starts) and frame-dependent behaviour is exercised naturally. The engine
//itself reports assertion failures through _test, so a test just calls them.

::_testSystem <- {

    mTests_ = []
    mIndex_ = 0
    //Frames to idle before the first test, so the plugin has rendered at least
    //one frame and things like getFramerate have real values.
    mWarmupFrames_ = 5

    function register(name, description, closure){
        mTests_.append({ name = name, description = description, closure = closure });
    }

    function update(){
        //The engine runs script updates on a fixed timestep, so several land in
        //one rendered frame. Only start a test at the top of a rendered frame,
        //which is what gives each test an imgui frame of its own.
        if(!_imgui.isFirstUpdateOfFrame()) return;

        if(mWarmupFrames_ > 0){
            mWarmupFrames_--;
            //Touch imgui so the frame is begun and consumed like any other.
            _imgui.getFrameCount();
            return;
        }

        if(mIndex_ >= mTests_.len()){
            printf("====== %i tests finished ======", mTests_.len());
            _test.endTest();
            return;
        }

        local t = mTests_[mIndex_];
        mIndex_++;
        printf("------ %s ------", t.name);
        t.closure();
    }
};

::_t <- function(name, description, closure){
    ::_testSystem.register(name, description, closure);
};

//Assert that calling `closure` raises a squirrel error. Used to check that the
//api rejects bad input rather than crashing the engine.
::_tThrows <- function(description, closure){
    local threw = false;
    try{
        closure();
    }catch(e){
        threw = true;
    }
    _test.assertTrue(threw);
};

//Assert a float is within a tolerance, for values that can't be exact.
::_tNear <- function(actual, expected, tolerance = 0.0001){
    local diff = actual - expected;
    if(diff < 0) diff = -diff;
    _test.assertTrue(diff <= tolerance);
};

function start(){
    local setupFile = _settings.getUserSetting("SetupFile");
    if(typeof setupFile != "string") throw "No test setup file was provided.";
    _doFile(setupFile);
}

function update(){
    ::_testSystem.update();
}

function end(){
}
