//Window creation, the next-window setters and the window queries.

_t("beginEnd", "begin returns a bool and pairs with end", function(){
    local open = _imgui.begin("windows/basic");
    _test.assertEqual("bool", typeof open);
    _imgui.end();
});

_t("beginClosable", "beginClosable returns visibility and open state as a pair", function(){
    local state = _imgui.beginClosable("windows/closable");
    _imgui.end();

    _test.assertEqual("array", typeof state);
    _test.assertEqual(2, state.len());
    _test.assertEqual("bool", typeof state[0]);
    //Nothing has clicked the close button, so the window is still open.
    _test.assertTrue(state[1]);
});

_t("beginClosableWithFlags", "beginClosable accepts window flags", function(){
    //A window with no title bar has nowhere to put the close button, which imgui
    //allows rather than treating as a contradiction.
    local state = _imgui.beginClosable("windows/closableFlags",
        _imgui.WindowFlags_NoTitleBar | _imgui.WindowFlags_NoResize);
    _imgui.end();

    _test.assertTrue(state[1]);
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

//Scrolling. imgui applies a scroll request at the window's next begin rather
//than at the call, so each of these is a pair: one frame asks, the next reads.
::_scrollChildRow <- null;
::_scrollChild <- function(){
    _imgui.begin("windows/scroll");
    local shown = _imgui.beginChild("scrollV", 200, 100, _imgui.ChildFlags_Borders);
    for(local i = 0; i < 200; i++){
        if(i == 100){
            //Where the row the later tests centre on sits, measured rather than
            //assumed so the check does not depend on the font.
            ::_scrollChildRow = {
                "top": _imgui.getCursorPosY(),
                "height": _imgui.calcTextSize("Row")[1],
                "windowHeight": _imgui.getWindowSize()[1],
                "scroll": _imgui.getScrollY()
            };
        }
        _imgui.text("Row " + i);
    }
    return shown;
};

_t("scrollQueries", "Scroll queries return floats and a set is accepted", function(){
    ::_scrollChild();
    local x = _imgui.getScrollX();
    local y = _imgui.getScrollY();
    local maxX = _imgui.getScrollMaxX();
    local maxY = _imgui.getScrollMaxY();
    _imgui.setScrollY(50);
    _imgui.endChild();
    _imgui.end();

    _test.assertEqual("float", typeof x);
    _test.assertEqual("float", typeof y);
    _test.assertEqual("float", typeof maxX);
    _test.assertEqual("float", typeof maxY);
    //A fresh window starts at the top.
    ::_tNear(y, 0.0);
});

_t("setScrollYApplied", "The scroll asked for last frame is in place this frame", function(){
    ::_scrollChild();
    local y = _imgui.getScrollY();
    local maxY = _imgui.getScrollMaxY();
    //Far past the end, to be clamped.
    _imgui.setScrollY(1000000);
    _imgui.endChild();
    _imgui.end();

    ::_tNear(y, 50.0, 0.5);
    //Two hundred rows in a hundred pixel child leave plenty to scroll.
    _test.assertTrue(maxY > 100.0);
});

_t("setScrollYClamped", "A scroll past the end stops at the maximum", function(){
    ::_scrollChild();
    local y = _imgui.getScrollY();
    local maxY = _imgui.getScrollMaxY();
    _imgui.setScrollY(0);
    _imgui.endChild();
    _imgui.end();

    ::_tNear(y, maxY, 0.5);
    _test.assertTrue(y > 100.0);
});

_t("setScrollHereYRequest", "setScrollHereY is asked for on the row to centre", function(){
    _imgui.begin("windows/scroll");
    _imgui.beginChild("scrollV", 200, 100, _imgui.ChildFlags_Borders);
    for(local i = 0; i < 200; i++){
        _imgui.text("Row " + i);
        if(i == 100) _imgui.setScrollHereY(0.5);
    }
    _imgui.endChild();
    _imgui.end();
});

_t("setScrollHereYCentres", "The row is in the middle of the child the frame after", function(){
    ::_scrollChild();
    local y = _imgui.getScrollY();
    _imgui.endChild();
    _imgui.end();

    //The row's middle sits at the middle of the child's visible area. The
    //measurements come from this frame, in which the child is already scrolled
    //to where the request put it, and top includes that scroll.
    local row = ::_scrollChildRow;
    local expected = row.top + row.height * 0.5 - row.windowHeight * 0.5;
    ::_tNear(y, expected, 1.5);
    _test.assertTrue(y > 0.0);
});

_t("setScrollFromPosYRequest", "setScrollFromPosY takes a window-relative position", function(){
    ::_scrollChild();
    //The row's top, relative to the window rather than to the content: the
    //cursor includes the scroll, so it comes back out. Ratio 0 puts it at the
    //top of the visible area.
    local row = ::_scrollChildRow;
    _imgui.setScrollFromPosY(row.top - row.scroll, 0.0);
    _imgui.endChild();
    _imgui.end();
});

_t("setScrollFromPosYApplied", "The position given is at the top the frame after", function(){
    ::_scrollChild();
    local y = _imgui.getScrollY();
    _imgui.endChild();
    _imgui.end();

    ::_tNear(y, ::_scrollChildRow.top, 1.5);
});

_t("setScrollXRequest", "Horizontal scrolling is asked for in a wide child", function(){
    _imgui.begin("windows/scroll");
    _imgui.beginChild("scrollH", 200, 60, _imgui.ChildFlags_Borders,
        _imgui.WindowFlags_HorizontalScrollbar);
    _imgui.dummy(1000, 10);
    _imgui.text("Wide");
    local x = _imgui.getScrollX();
    _imgui.setScrollX(30);
    _imgui.endChild();
    _imgui.end();

    ::_tNear(x, 0.0);
});

_t("setScrollXApplied", "The horizontal scroll is in place the frame after", function(){
    _imgui.begin("windows/scroll");
    _imgui.beginChild("scrollH", 200, 60, _imgui.ChildFlags_Borders,
        _imgui.WindowFlags_HorizontalScrollbar);
    _imgui.dummy(1000, 10);
    _imgui.text("Wide");
    local x = _imgui.getScrollX();
    local maxX = _imgui.getScrollMaxX();
    _imgui.endChild();
    _imgui.end();

    ::_tNear(x, 30.0, 0.5);
    _test.assertTrue(maxX > 500.0);
});
