//Window creation, the next-window setters and the window queries.

_t("beginEnd", "begin returns a bool and pairs with end", function(){
    local open = _imgui.begin("windows/basic");
    _test.assertEqual("bool", typeof open);
    _imgui.end();
});

_t("beginWithFlags", "begin accepts window flags", function(){
    local flags = _imgui.WindowFlags_NoTitleBar
                | _imgui.WindowFlags_NoResize
                | _imgui.WindowFlags_NoMove
                | _imgui.WindowFlags_AlwaysAutoResize;
    _imgui.begin("windows/flags", flags);
    _imgui.end();
});

_t("setNextWindowCollapsed", "Collapsing is applied on the frame it is requested", function(){
    _imgui.setNextWindowCollapsed(true, _imgui.Cond_Always);
    _imgui.begin("windows/collapsed");
    local collapsed = _imgui.isWindowCollapsed();
    _imgui.end();
    _test.assertTrue(collapsed);
    //Note imgui still returns true from begin on the very first frame a window
    //is submitted, even when collapsed; the next test covers the frame after.
});

_t("beginCollapsedReturn", "An established collapsed window returns false from begin", function(){
    //windows/collapsed was created and collapsed by the previous test, one
    //frame ago, so begin now reports that its contents are skipped.
    _imgui.setNextWindowCollapsed(true, _imgui.Cond_Always);
    local open = _imgui.begin("windows/collapsed");
    _imgui.end();
    _test.assertFalse(open);

    //Leave it expanded again.
    _imgui.setNextWindowCollapsed(false, _imgui.Cond_Always);
    _imgui.begin("windows/collapsed");
    _imgui.end();
});

_t("beginChild", "beginChild works with default and explicit arguments", function(){
    _imgui.begin("windows/child");

    local shown = _imgui.beginChild("childDefaults");
    _test.assertEqual("bool", typeof shown);
    _imgui.endChild();

    _imgui.beginChild("childSized", 120, 80);
    _imgui.endChild();

    _imgui.beginChild("childFlags", 120, 80, _imgui.ChildFlags_Borders, _imgui.WindowFlags_NoScrollbar);
    _imgui.endChild();

    _imgui.end();
});

_t("setNextWindowPos", "Position is applied, with and without the optional arguments", function(){
    _imgui.setNextWindowPos(40, 60, _imgui.Cond_Always);
    _imgui.begin("windows/pos");
    local pos = _imgui.getWindowPos();
    _imgui.end();

    _test.assertEqual("array", typeof pos);
    _test.assertEqual(2, pos.len());
    ::_tNear(pos[0], 40.0, 0.5);
    ::_tNear(pos[1], 60.0, 0.5);

    //Pivot arguments are accepted. Their effect depends on the window size,
    //which imgui only knows once the window has been submitted for a frame, so
    //this just checks the call path here; beginCollapsedReturn's neighbour test
    //covers second-frame positioning.
    _imgui.setNextWindowPos(200, 200, _imgui.Cond_Always, 0.5, 0.5);
    _imgui.begin("windows/posPivot");
    _imgui.end();

    //No condition argument at all.
    _imgui.setNextWindowPos(10, 10);
    _imgui.begin("windows/posNoCond");
    _imgui.end();
});

_t("setNextWindowPosPivotSettled", "A centre pivot centres a window whose size is known", function(){
    //windows/posPivot was submitted once by the previous test, so its size is
    //now known and the centre pivot lands its middle on the point.
    _imgui.setNextWindowPos(200, 200, _imgui.Cond_Always, 0.5, 0.5);
    _imgui.begin("windows/posPivot");
    local pos = _imgui.getWindowPos();
    local size = _imgui.getWindowSize();
    _imgui.end();
    ::_tNear(pos[0] + size[0] * 0.5, 200.0, 1.0);
    ::_tNear(pos[1] + size[1] * 0.5, 200.0, 1.0);
});

_t("setNextWindowSize", "Size is applied and reported back", function(){
    _imgui.setNextWindowSize(320, 240, _imgui.Cond_Always);
    _imgui.begin("windows/size");
    local size = _imgui.getWindowSize();
    local w = _imgui.getWindowWidth();
    local h = _imgui.getWindowHeight();
    _imgui.end();

    ::_tNear(size[0], 320.0, 0.5);
    ::_tNear(size[1], 240.0, 0.5);
    _test.assertEqual("float", typeof w);
    _test.assertEqual("float", typeof h);
    ::_tNear(w, 320.0, 0.5);
    ::_tNear(h, 240.0, 0.5);

    //Without the optional condition.
    _imgui.setNextWindowSize(100, 100);
    _imgui.begin("windows/sizeNoCond");
    _imgui.end();
});

_t("setNextWindowBgAlpha", "Background alpha is accepted", function(){
    _imgui.setNextWindowBgAlpha(0.25);
    _imgui.begin("windows/alpha");
    _imgui.end();
});

_t("setNextWindowFocus", "Focus request is accepted", function(){
    _imgui.setNextWindowFocus();
    _imgui.begin("windows/focus");
    _imgui.end();
});

_t("windowQueries", "The window state queries return booleans", function(){
    _imgui.begin("windows/queries");
    local hovered = _imgui.isWindowHovered();
    local focused = _imgui.isWindowFocused();
    local collapsed = _imgui.isWindowCollapsed();
    //With flags.
    local hoveredFlags = _imgui.isWindowHovered(_imgui.HoveredFlags_ChildWindows);
    local focusedFlags = _imgui.isWindowFocused(_imgui.FocusedFlags_RootWindow);
    _imgui.end();

    _test.assertEqual("bool", typeof hovered);
    _test.assertEqual("bool", typeof focused);
    _test.assertEqual("bool", typeof collapsed);
    _test.assertEqual("bool", typeof hoveredFlags);
    _test.assertEqual("bool", typeof focusedFlags);
});

_t("getContentRegionAvail", "Content region is a positive pair inside a sized window", function(){
    _imgui.setNextWindowSize(300, 200, _imgui.Cond_Always);
    _imgui.begin("windows/content");
    local avail = _imgui.getContentRegionAvail();
    _imgui.end();

    _test.assertEqual("array", typeof avail);
    _test.assertEqual(2, avail.len());
    _test.assertTrue(avail[0] > 0.0);
    _test.assertTrue(avail[1] > 0.0);
    //It must fit inside the window it came from.
    _test.assertTrue(avail[0] <= 300.0);
    _test.assertTrue(avail[1] <= 200.0);
});
