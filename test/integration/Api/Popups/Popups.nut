//Popups and tooltips. A popup only becomes visible once it has been opened,
//which lets these assert real state changes rather than just "didn't crash".
//
//isPopupOpen is scoped to the current window's id stack (unless the AnyPopupId
//flag is given), so a popup opened inside a window is queried inside that same
//window.

_t("popupClosedByDefault", "An unopened popup is not open and does not begin", function(){
    _imgui.begin("popups/closed");
    _test.assertFalse(_imgui.isPopupOpen("neverOpened"));
    local shown = _imgui.beginPopup("neverOpened");
    _test.assertEqual("bool", typeof shown);
    _test.assertFalse(shown);
    if(shown) _imgui.endPopup();
    _imgui.end();
});

_t("openAndShowPopup", "A popup opens, becomes visible, and can close itself", function(){
    //Open and query within the same window and frame.
    _imgui.begin("popups/open");
    _imgui.openPopup("testPopup");
    _test.assertTrue(_imgui.isPopupOpen("testPopup"));

    local shown = _imgui.beginPopup("testPopup");
    _test.assertEqual("bool", typeof shown);
    _test.assertTrue(shown);
    if(shown){
        _imgui.text("popup contents");
        _imgui.button("a button in a popup");
        _imgui.closeCurrentPopup();
        _imgui.endPopup();
    }
    _imgui.end();
});

_t("popupClosedAgain", "closeCurrentPopup actually closed it", function(){
    _imgui.begin("popups/open");
    _test.assertFalse(_imgui.isPopupOpen("testPopup"));
    _imgui.end();
});

_t("openPopupFlags", "openPopup and isPopupOpen accept their flag arguments", function(){
    _imgui.begin("popups/flags");
    _imgui.openPopup("flagPopup", _imgui.PopupFlags_NoOpenOverExistingPopup);
    _test.assertTrue(_imgui.isPopupOpen("flagPopup", _imgui.PopupFlags_None));
    //AnyPopupId ignores the identifier, so any open popup satisfies it.
    _test.assertTrue(_imgui.isPopupOpen("", _imgui.PopupFlags_AnyPopupId));

    local shown = _imgui.beginPopup("flagPopup", _imgui.WindowFlags_NoMove);
    if(shown){
        _imgui.text("flagged popup");
        _imgui.closeCurrentPopup();
        _imgui.endPopup();
    }
    _imgui.end();
});

_t("popupModal", "A modal popup opens and closes", function(){
    _imgui.begin("popups/modalOwner");
    _imgui.openPopup("modalPopup");
    _imgui.end();

    local shown = _imgui.beginPopupModal("modalPopup");
    _test.assertEqual("bool", typeof shown);
    if(shown){
        _imgui.text("modal contents");
        _imgui.closeCurrentPopup();
        _imgui.endPopup();
    }
});

_t("popupModalFlags", "A modal popup accepts window flags", function(){
    _imgui.begin("popups/modalFlagOwner");
    _imgui.openPopup("modalFlagged");
    _imgui.end();

    local shown = _imgui.beginPopupModal("modalFlagged", _imgui.WindowFlags_AlwaysAutoResize | _imgui.WindowFlags_NoTitleBar);
    if(shown){
        _imgui.text("modal contents");
        _imgui.closeCurrentPopup();
        _imgui.endPopup();
    }
});

_t("setTooltip", "The one-shot tooltip is accepted, including format characters", function(){
    _imgui.begin("popups/tooltip");
    _imgui.text("hover me");
    _imgui.setTooltip("a tooltip");
    _imgui.setTooltip("100% literal %s");
    _imgui.end();
});

_t("beginTooltip", "An explicit tooltip block opens and closes", function(){
    _imgui.begin("popups/tooltipBlock");
    local shown = _imgui.beginTooltip();
    _test.assertEqual("bool", typeof shown);
    if(shown){
        _imgui.text("tooltip line one");
        _imgui.text("tooltip line two");
        _imgui.endTooltip();
    }
    _imgui.end();
});

_t("beginItemTooltip", "The item tooltip only opens when the item is hovered", function(){
    _imgui.begin("popups/itemTooltip");
    _imgui.button("notHovered");
    local shown = _imgui.beginItemTooltip();
    _test.assertEqual("bool", typeof shown);
    //Nothing is hovered during the test.
    _test.assertFalse(shown);
    if(shown) _imgui.endTooltip();
    _imgui.end();
});
