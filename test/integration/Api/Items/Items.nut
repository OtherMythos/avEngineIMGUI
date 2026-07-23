//Item state queries. Nothing is interacted with, so all of them must report
//false rather than, say, uninitialised memory.

_t("itemQueryTypes", "Every item query returns a bool", function(){
    _imgui.begin("items/types");
    _imgui.button("queried");

    _test.assertEqual("bool", typeof _imgui.isItemHovered());
    _test.assertEqual("bool", typeof _imgui.isItemActive());
    _test.assertEqual("bool", typeof _imgui.isItemClicked());
    _test.assertEqual("bool", typeof _imgui.isItemEdited());
    _test.assertEqual("bool", typeof _imgui.isItemActivated());
    _test.assertEqual("bool", typeof _imgui.isItemDeactivated());
    _test.assertEqual("bool", typeof _imgui.isItemDeactivatedAfterEdit());
    _test.assertEqual("bool", typeof _imgui.isAnyItemActive());
    _test.assertEqual("bool", typeof _imgui.isAnyItemHovered());

    _imgui.end();
});

_t("untouchedItemsAreInactive", "An untouched item reports no interaction", function(){
    _imgui.begin("items/inactive");
    _imgui.button("untouched");

    _test.assertFalse(_imgui.isItemActive());
    _test.assertFalse(_imgui.isItemClicked());
    _test.assertFalse(_imgui.isItemEdited());
    _test.assertFalse(_imgui.isItemActivated());
    _test.assertFalse(_imgui.isItemDeactivated());
    _test.assertFalse(_imgui.isItemDeactivatedAfterEdit());
    _test.assertFalse(_imgui.isAnyItemActive());

    _imgui.end();
});

_t("itemQueryFlags", "The flag arguments are accepted", function(){
    _imgui.begin("items/flags");
    _imgui.button("flagged");

    _test.assertEqual("bool", typeof _imgui.isItemHovered(_imgui.HoveredFlags_AllowWhenDisabled));
    _test.assertEqual("bool", typeof _imgui.isItemHovered(_imgui.HoveredFlags_AllowWhenBlockedByPopup));
    _test.assertEqual("bool", typeof _imgui.isItemHovered(_imgui.HoveredFlags_RectOnly));

    //Every mouse button, by constant and by raw index.
    _test.assertFalse(_imgui.isItemClicked(_imgui.MouseButton_Left));
    _test.assertFalse(_imgui.isItemClicked(_imgui.MouseButton_Right));
    _test.assertFalse(_imgui.isItemClicked(_imgui.MouseButton_Middle));
    _test.assertFalse(_imgui.isItemClicked(0));

    _imgui.end();
});

_t("setItemDefaultFocus", "Default focus can be requested for an item", function(){
    _imgui.begin("items/focus");
    _imgui.button("focusTarget");
    _imgui.setItemDefaultFocus();
    _imgui.end();
});

_t("setKeyboardFocusHere", "Keyboard focus accepts an optional offset", function(){
    _imgui.begin("items/keyboardFocus");
    //No argument, and an explicit offset.
    _imgui.setKeyboardFocusHere();
    _imgui.button("focusA");
    _imgui.setKeyboardFocusHere(0);
    _imgui.button("focusB");
    _imgui.end();
});

_t("hoveredWindowIsFalse", "Nothing is hovered while the test runs", function(){
    _imgui.begin("items/hover");
    _imgui.button("notHovered");
    _test.assertFalse(_imgui.isItemHovered());
    _test.assertFalse(_imgui.isAnyItemHovered());
    _imgui.end();
});
