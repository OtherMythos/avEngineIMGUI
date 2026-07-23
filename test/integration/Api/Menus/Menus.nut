//Menu bars, menus and tab bars. imgui's rule is that the matching end* is only
//called when the begin returned true, so these follow that exactly.

_t("windowMenuBar", "A window menu bar opens when the window asks for one", function(){
    _imgui.begin("menus/bar", _imgui.WindowFlags_MenuBar);
    local opened = _imgui.beginMenuBar();
    _test.assertEqual("bool", typeof opened);
    _test.assertTrue(opened);
    if(opened){
        local menuOpen = _imgui.beginMenu("File");
        _test.assertEqual("bool", typeof menuOpen);
        //Not hovered, so the menu stays closed.
        _test.assertFalse(menuOpen);
        if(menuOpen) _imgui.endMenu();
        _imgui.endMenuBar();
    }
    _imgui.end();
});

_t("menuBarWithoutFlag", "A window without the flag has no menu bar", function(){
    _imgui.begin("menus/noBar");
    local opened = _imgui.beginMenuBar();
    _test.assertFalse(opened);
    if(opened) _imgui.endMenuBar();
    _imgui.end();
});

_t("mainMenuBar", "The main menu bar opens and closes", function(){
    local opened = _imgui.beginMainMenuBar();
    _test.assertEqual("bool", typeof opened);
    if(opened){
        //beginMenu's enabled argument is optional.
        local fileOpen = _imgui.beginMenu("File");
        if(fileOpen) _imgui.endMenu();

        local disabledOpen = _imgui.beginMenu("Disabled", false);
        _test.assertFalse(disabledOpen);
        if(disabledOpen) _imgui.endMenu();

        _imgui.endMainMenuBar();
    }
});

_t("menuItem", "Menu items report activation and accept every argument", function(){
    //Menu items are legal outside a menu; they draw as ordinary rows.
    _imgui.begin("menus/items");

    local clicked = _imgui.menuItem("plain");
    _test.assertEqual("bool", typeof clicked);
    _test.assertFalse(clicked);

    _test.assertFalse(_imgui.menuItem("withShortcut", "Ctrl+S"));
    //The shortcut is optional and documented as accepting null.
    _test.assertFalse(_imgui.menuItem("nullShortcut", null));
    _test.assertFalse(_imgui.menuItem("selected", "Ctrl+T", true));
    _test.assertFalse(_imgui.menuItem("disabled", "Ctrl+D", false, false));

    _imgui.end();
});

_t("tabBar", "Tab bars open, and the first tab is selected by default", function(){
    _imgui.begin("menus/tabs");

    local barOpen = _imgui.beginTabBar("tabBar");
    _test.assertEqual("bool", typeof barOpen);
    _test.assertTrue(barOpen);

    if(barOpen){
        local first = _imgui.beginTabItem("first");
        _test.assertEqual("bool", typeof first);
        //Nothing else has been selected, so the first tab is active.
        _test.assertTrue(first);
        if(first){
            _imgui.text("first tab contents");
            _imgui.endTabItem();
        }

        local second = _imgui.beginTabItem("second");
        _test.assertFalse(second);
        if(second) _imgui.endTabItem();

        _imgui.endTabBar();
    }

    _imgui.end();
});

_t("tabBarFlags", "Tab bars and items accept their flags", function(){
    _imgui.begin("menus/tabFlags");

    local flags = _imgui.TabBarFlags_Reorderable
                | _imgui.TabBarFlags_NoTooltip
                | _imgui.TabBarFlags_FittingPolicyScroll;
    if(_imgui.beginTabBar("flaggedBar", flags)){
        local item = _imgui.beginTabItem("flaggedItem", _imgui.TabItemFlags_UnsavedDocument);
        if(item) _imgui.endTabItem();

        local trailing = _imgui.beginTabItem("trailingItem", _imgui.TabItemFlags_Trailing);
        if(trailing) _imgui.endTabItem();

        _imgui.endTabBar();
    }

    _imgui.end();
});
