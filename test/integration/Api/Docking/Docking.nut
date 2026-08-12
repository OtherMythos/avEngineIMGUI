//The docking api, from the docking branch of imgui.
//
//Docking is stateful across frames: a dock node lives in the imgui context and
//is dropped again if the dockspace which owns it stops being submitted. The
//harness runs one test per rendered frame, so tests which need a settled node
//come in pairs, the same way the Windows tests do for collapsing.
//
//Two imgui rules the tests here have to respect, because breaking either raises
//an imgui assertion that stops the engine rather than a recoverable error:
//  - The same dockspace id may only be submitted once a frame, unless the
//    submission uses DockNodeFlags_KeepAliveOnly.
//  - A window docked into a dockspace must be submitted after the window which
//    hosts that dockspace, or imgui undocks it again.

//A dockspace must be re-submitted every frame or imgui drops the node and
//undocks its windows, so every test that uses one builds it through here.
//The returned id is what identifies the node to setNextWindowDockId.
::dockHost <- function(){
    _imgui.setNextWindowPos(0, 0, _imgui.Cond_Always);
    _imgui.setNextWindowSize(400, 300, _imgui.Cond_Always);
    //The host of a dockspace must not itself be dockable.
    _imgui.begin("docking/host", _imgui.WindowFlags_NoDocking);
    local id = _imgui.dockSpace("space");
    _imgui.end();
    return id;
}

//Ids captured on one frame so a later frame can compare against them.
::dockSpaceId <- 0;
::byIntId <- 0;
::overViewportId <- 0;

_t("dockingEnabledByDefault", "Docking is on without the script asking for it", function(){
    _test.assertTrue(_imgui.getDockingEnabled());
});

_t("dockSpaceReturnsId", "dockSpace returns a non-zero integer node id", function(){
    local id = ::dockHost();
    _test.assertEqual("integer", typeof id);
    _test.assertNotEqual(0, id);
    ::dockSpaceId = id;
});

_t("dockSpaceIdIsStable", "The same dockspace yields the same id on a later frame", function(){
    local id = ::dockHost();
    _test.assertEqual(::dockSpaceId, id);
});

_t("dockSpaceFlags", "dockSpace accepts a size and node flags", function(){
    _imgui.begin("docking/flags", _imgui.WindowFlags_NoDocking);
    local id = _imgui.dockSpace("sized", 200, 150,
        _imgui.DockNodeFlags_NoResize | _imgui.DockNodeFlags_AutoHideTabBar);
    _imgui.end();
    _test.assertNotEqual(0, id);
});

_t("dockSpaceKeepAliveOnly", "A hidden dockspace can be kept alive without being drawn", function(){
    //KeepAliveOnly is how a dockspace inside a hidden tab keeps its windows
    //docked. It needs no visible node, so it draws nothing.
    _imgui.begin("docking/keepAlive", _imgui.WindowFlags_NoDocking);
    local id = _imgui.dockSpace("kept", 0, 0, _imgui.DockNodeFlags_KeepAliveOnly);
    _imgui.end();
    _test.assertEqual("integer", typeof id);
    _test.assertNotEqual(0, id);
});

_t("dockSpaceIdForIntegerUse", "A dockspace id is captured for the next test", function(){
    _imgui.begin("docking/byInt", _imgui.WindowFlags_NoDocking);
    ::byIntId = _imgui.dockSpace("byIntSpace");
    _imgui.end();
    _test.assertNotEqual(0, ::byIntId);
});

_t("dockSpaceTakesAnIntegerId", "An id from a previous call can be passed straight back", function(){
    //The integer form exists so an id can travel between calls: passing the
    //captured id names the same node the string named last frame.
    _imgui.begin("docking/byInt", _imgui.WindowFlags_NoDocking);
    local id = _imgui.dockSpace(::byIntId);
    _imgui.end();
    _test.assertEqual(::byIntId, id);
});

_t("dockSpaceOverViewport", "dockSpaceOverViewport covers the display and returns an id", function(){
    local id = _imgui.dockSpaceOverViewport();
    _test.assertEqual("integer", typeof id);
    _test.assertNotEqual(0, id);
});

_t("dockSpaceOverViewportNamed", "dockSpaceOverViewport takes an explicit id and node flags", function(){
    //PassthruCentralNode is the usual choice for a full screen dockspace: the
    //empty middle stays transparent so the game is still visible through it.
    ::overViewportId = _imgui.dockSpaceOverViewport("overViewport",
        _imgui.DockNodeFlags_PassthruCentralNode);
    _test.assertEqual("integer", typeof ::overViewportId);
    _test.assertNotEqual(0, ::overViewportId);
});

_t("dockSpaceOverViewportById", "The id it returned names the same dockspace", function(){
    local again = _imgui.dockSpaceOverViewport(::overViewportId,
        _imgui.DockNodeFlags_PassthruCentralNode);
    _test.assertEqual(::overViewportId, again);
});

_t("setNextWindowDockId", "A window submitted into a dockspace takes its dock id", function(){
    ::dockHost();

    //Submitted after the host window, as imgui requires.
    _imgui.setNextWindowDockId(::dockSpaceId, _imgui.Cond_Always);
    _imgui.begin("docking/docked");
    local dockId = _imgui.getWindowDockId();
    local docked = _imgui.isWindowDocked();
    _imgui.end();

    _test.assertEqual("integer", typeof dockId);
    _test.assertEqual(::dockSpaceId, dockId);
    _test.assertEqual("bool", typeof docked);
});

_t("isWindowDockedSettled", "The docked window reports itself docked once the node is established", function(){
    //docking/docked was docked by the previous test, a frame ago, so the node
    //now has a host window and the window is really docked into it.
    ::dockHost();

    _imgui.begin("docking/docked");
    local docked = _imgui.isWindowDocked();
    local dockId = _imgui.getWindowDockId();
    _imgui.end();

    _test.assertTrue(docked);
    //The dock id survives without setNextWindowDockId being called again.
    _test.assertEqual(::dockSpaceId, dockId);
});

_t("undockedWindow", "A window outside any dockspace is not docked", function(){
    ::dockHost();

    _imgui.begin("docking/floating");
    local docked = _imgui.isWindowDocked();
    local dockId = _imgui.getWindowDockId();
    _imgui.end();

    _test.assertFalse(docked);
    _test.assertEqual(0, dockId);
});

_t("noDockingWindowFlag", "WindowFlags_NoDocking keeps a window out of a dockspace", function(){
    ::dockHost();

    //imgui undocks a NoDocking window even when a dock id is requested for it.
    _imgui.setNextWindowDockId(::dockSpaceId, _imgui.Cond_Always);
    _imgui.begin("docking/noDocking", _imgui.WindowFlags_NoDocking);
    local docked = _imgui.isWindowDocked();
    _imgui.end();

    _test.assertFalse(docked);
});

_t("setNextWindowDockIdTakesAString", "The dock id may be given as a string", function(){
    ::dockHost();

    //A string is hashed the way imgui hashes any id. Hashed here at the top
    //level rather than inside the host window, so it is its own id and not the
    //dockspace's; this only has to be a call that works.
    _imgui.setNextWindowDockId("someDock", _imgui.Cond_FirstUseEver);
    _imgui.begin("docking/stringId");
    local dockId = _imgui.getWindowDockId();
    _imgui.end();

    _test.assertEqual("integer", typeof dockId);
    _test.assertNotEqual(0, dockId);
});

_t("getId", "getId hashes a string into an integer id", function(){
    _imgui.begin("docking/ids");
    local a = _imgui.getId("first");
    local aAgain = _imgui.getId("first");
    local b = _imgui.getId("second");
    _imgui.end();

    _test.assertEqual("integer", typeof a);
    //The same string in the same place is the same id.
    _test.assertEqual(a, aAgain);
    //Different strings are different ids.
    _test.assertNotEqual(a, b);
});

_t("getIdIsRelativeToTheWindow", "The same string in two windows gives two ids", function(){
    //imgui hashes an id against the current window and id stack, which is why
    //dockSpace returns its id rather than expecting scripts to re-derive it.
    _imgui.begin("docking/idsA");
    local inA = _imgui.getId("shared");
    _imgui.end();

    _imgui.begin("docking/idsB");
    local inB = _imgui.getId("shared");
    _imgui.end();

    _test.assertNotEqual(inA, inB);
});

_t("dockingConfigToggles", "The docking config options round-trip", function(){
    _test.assertEqual("bool", typeof _imgui.getDockingNoSplit());
    _test.assertEqual("bool", typeof _imgui.getDockingWithShift());
    _test.assertEqual("bool", typeof _imgui.getDockingAlwaysTabBar());

    _imgui.setDockingNoSplit(true);
    _test.assertTrue(_imgui.getDockingNoSplit());
    _imgui.setDockingNoSplit(false);
    _test.assertFalse(_imgui.getDockingNoSplit());

    _imgui.setDockingWithShift(true);
    _test.assertTrue(_imgui.getDockingWithShift());
    _imgui.setDockingWithShift(false);
    _test.assertFalse(_imgui.getDockingWithShift());

    _imgui.setDockingAlwaysTabBar(true);
    _test.assertTrue(_imgui.getDockingAlwaysTabBar());
    _imgui.setDockingAlwaysTabBar(false);
    _test.assertFalse(_imgui.getDockingAlwaysTabBar());
});

_t("dockingErrors", "Bad docking calls raise an error rather than an imgui assertion", function(){
    _imgui.begin("docking/errors");

    //imgui asserts on a zero dockspace id, which would stop the engine.
    ::_tThrows("dockSpace with a zero id", function(){
        _imgui.dockSpace(0);
    });
    ::_tThrows("dockSpace with no id", function(){
        _imgui.dockSpace();
    });
    ::_tThrows("dockSpace with a table for an id", function(){
        _imgui.dockSpace({});
    });
    ::_tThrows("setNextWindowDockId with no arguments", function(){
        _imgui.setNextWindowDockId();
    });
    ::_tThrows("setDockingEnabled with a string", function(){
        _imgui.setDockingEnabled("yes");
    });
    ::_tThrows("getWindowDockId with a spare argument", function(){
        _imgui.getWindowDockId(1);
    });

    _imgui.end();

    //The api is still usable afterwards.
    _test.assertNotEqual(0, ::dockHost());
});

_t("dockingDisabled", "With docking off the dockspace calls are refused, not silently wrong", function(){
    //Left until last: turning docking off undocks everything.
    _imgui.setDockingEnabled(false);
    _test.assertFalse(_imgui.getDockingEnabled());

    //begin/end stay paired around the rejected call: an unmatched begin is an
    //imgui assertion, so the throw has to happen between them, not across them.
    _imgui.begin("docking/disabled");
    ::_tThrows("dockSpace while docking is disabled", function(){
        _imgui.dockSpace("space");
    });
    _imgui.end();

    ::_tThrows("dockSpaceOverViewport while docking is disabled", function(){
        _imgui.dockSpaceOverViewport();
    });

    //The non-dockspace queries keep working, they just report nothing docked.
    _imgui.begin("docking/disabledQueries");
    local docked = _imgui.isWindowDocked();
    _imgui.end();
    _test.assertFalse(docked);

    _imgui.setDockingEnabled(true);
    _test.assertTrue(_imgui.getDockingEnabled());
});
