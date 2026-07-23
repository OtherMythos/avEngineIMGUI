//Every constant the plugin registers. Generated from ImguiConstants.cpp,
//so a constant added there without a test shows up as a missing name here.

::checkConstants <- function(groupName, names){
    foreach(n in names){
        if(!(n in _imgui)){
            throw format("_imgui.%s is missing", n);
        }
        _test.assertEqual("integer", typeof _imgui[n]);
    }
}

_t("WindowFlags", "Every WindowFlags constant is exposed as an integer", function(){
    ::checkConstants("WindowFlags", [
        "WindowFlags_None", "WindowFlags_NoTitleBar", "WindowFlags_NoResize",
        "WindowFlags_NoMove", "WindowFlags_NoScrollbar", "WindowFlags_NoScrollWithMouse",
        "WindowFlags_NoCollapse", "WindowFlags_AlwaysAutoResize", "WindowFlags_NoBackground",
        "WindowFlags_NoSavedSettings", "WindowFlags_NoMouseInputs", "WindowFlags_MenuBar",
        "WindowFlags_HorizontalScrollbar", "WindowFlags_NoFocusOnAppearing", "WindowFlags_NoBringToFrontOnFocus",
        "WindowFlags_AlwaysVerticalScrollbar", "WindowFlags_AlwaysHorizontalScrollbar", "WindowFlags_NoNavInputs",
        "WindowFlags_NoNavFocus", "WindowFlags_UnsavedDocument", "WindowFlags_NoNav",
        "WindowFlags_NoDecoration", "WindowFlags_NoInputs"
    ]);
});

_t("ChildFlags", "Every ChildFlags constant is exposed as an integer", function(){
    ::checkConstants("ChildFlags", [
        "ChildFlags_None", "ChildFlags_Borders", "ChildFlags_AlwaysUseWindowPadding",
        "ChildFlags_ResizeX", "ChildFlags_ResizeY", "ChildFlags_AutoResizeX",
        "ChildFlags_AutoResizeY", "ChildFlags_AlwaysAutoResize", "ChildFlags_FrameStyle"
    ]);
});

_t("Cond", "Every Cond constant is exposed as an integer", function(){
    ::checkConstants("Cond", [
        "Cond_None", "Cond_Always", "Cond_Once",
        "Cond_FirstUseEver", "Cond_Appearing"
    ]);
});

_t("Dir", "Every Dir constant is exposed as an integer", function(){
    ::checkConstants("Dir", [
        "Dir_Left", "Dir_Right", "Dir_Up",
        "Dir_Down"
    ]);
});

_t("Col", "Every Col constant is exposed as an integer", function(){
    ::checkConstants("Col", [
        "Col_Text", "Col_TextDisabled", "Col_WindowBg",
        "Col_ChildBg", "Col_PopupBg", "Col_Border",
        "Col_FrameBg", "Col_FrameBgHovered", "Col_FrameBgActive",
        "Col_TitleBg", "Col_TitleBgActive", "Col_TitleBgCollapsed",
        "Col_MenuBarBg", "Col_ScrollbarBg", "Col_CheckMark",
        "Col_SliderGrab", "Col_SliderGrabActive", "Col_Button",
        "Col_ButtonHovered", "Col_ButtonActive", "Col_Header",
        "Col_HeaderHovered", "Col_HeaderActive", "Col_Separator",
        "Col_PlotLines", "Col_PlotHistogram", "Col_TableHeaderBg",
        "Col_TableRowBg", "Col_TableRowBgAlt"
    ]);
});

_t("StyleVar", "Every StyleVar constant is exposed as an integer", function(){
    ::checkConstants("StyleVar", [
        "StyleVar_Alpha", "StyleVar_DisabledAlpha", "StyleVar_WindowPadding",
        "StyleVar_WindowRounding", "StyleVar_WindowBorderSize", "StyleVar_WindowMinSize",
        "StyleVar_WindowTitleAlign", "StyleVar_ChildRounding", "StyleVar_ChildBorderSize",
        "StyleVar_PopupRounding", "StyleVar_PopupBorderSize", "StyleVar_FramePadding",
        "StyleVar_FrameRounding", "StyleVar_FrameBorderSize", "StyleVar_ItemSpacing",
        "StyleVar_ItemInnerSpacing", "StyleVar_IndentSpacing", "StyleVar_CellPadding",
        "StyleVar_ScrollbarSize", "StyleVar_ScrollbarRounding", "StyleVar_GrabMinSize",
        "StyleVar_GrabRounding", "StyleVar_TabRounding", "StyleVar_ButtonTextAlign",
        "StyleVar_SelectableTextAlign", "StyleVar_SeparatorTextBorderSize", "StyleVar_SeparatorTextAlign",
        "StyleVar_SeparatorTextPadding"
    ]);
});

_t("TreeNodeFlags", "Every TreeNodeFlags constant is exposed as an integer", function(){
    ::checkConstants("TreeNodeFlags", [
        "TreeNodeFlags_None", "TreeNodeFlags_Selected", "TreeNodeFlags_Framed",
        "TreeNodeFlags_AllowOverlap", "TreeNodeFlags_NoTreePushOnOpen", "TreeNodeFlags_DefaultOpen",
        "TreeNodeFlags_OpenOnDoubleClick", "TreeNodeFlags_OpenOnArrow", "TreeNodeFlags_Leaf",
        "TreeNodeFlags_Bullet", "TreeNodeFlags_FramePadding", "TreeNodeFlags_SpanAvailWidth",
        "TreeNodeFlags_SpanFullWidth", "TreeNodeFlags_CollapsingHeader"
    ]);
});

_t("InputTextFlags", "Every InputTextFlags constant is exposed as an integer", function(){
    ::checkConstants("InputTextFlags", [
        "InputTextFlags_None", "InputTextFlags_CharsDecimal", "InputTextFlags_CharsHexadecimal",
        "InputTextFlags_CharsScientific", "InputTextFlags_CharsUppercase", "InputTextFlags_CharsNoBlank",
        "InputTextFlags_AllowTabInput", "InputTextFlags_EnterReturnsTrue", "InputTextFlags_EscapeClearsAll",
        "InputTextFlags_CtrlEnterForNewLine", "InputTextFlags_ReadOnly", "InputTextFlags_Password",
        "InputTextFlags_AlwaysOverwrite", "InputTextFlags_AutoSelectAll", "InputTextFlags_NoHorizontalScroll",
        "InputTextFlags_NoUndoRedo"
    ]);
});

_t("SelectableFlags", "Every SelectableFlags constant is exposed as an integer", function(){
    ::checkConstants("SelectableFlags", [
        "SelectableFlags_None", "SelectableFlags_NoAutoClosePopups", "SelectableFlags_SpanAllColumns",
        "SelectableFlags_AllowDoubleClick", "SelectableFlags_Disabled", "SelectableFlags_AllowOverlap"
    ]);
});

_t("ComboFlags", "Every ComboFlags constant is exposed as an integer", function(){
    ::checkConstants("ComboFlags", [
        "ComboFlags_None", "ComboFlags_PopupAlignLeft", "ComboFlags_HeightSmall",
        "ComboFlags_HeightRegular", "ComboFlags_HeightLarge", "ComboFlags_HeightLargest",
        "ComboFlags_NoArrowButton", "ComboFlags_NoPreview", "ComboFlags_WidthFitPreview"
    ]);
});

_t("TableFlags", "Every TableFlags constant is exposed as an integer", function(){
    ::checkConstants("TableFlags", [
        "TableFlags_None", "TableFlags_Resizable", "TableFlags_Reorderable",
        "TableFlags_Hideable", "TableFlags_Sortable", "TableFlags_NoSavedSettings",
        "TableFlags_ContextMenuInBody", "TableFlags_RowBg", "TableFlags_BordersInnerH",
        "TableFlags_BordersOuterH", "TableFlags_BordersInnerV", "TableFlags_BordersOuterV",
        "TableFlags_BordersH", "TableFlags_BordersV", "TableFlags_BordersInner",
        "TableFlags_BordersOuter", "TableFlags_Borders", "TableFlags_SizingFixedFit",
        "TableFlags_SizingFixedSame", "TableFlags_SizingStretchProp", "TableFlags_SizingStretchSame",
        "TableFlags_NoHostExtendX", "TableFlags_NoHostExtendY", "TableFlags_ScrollX",
        "TableFlags_ScrollY"
    ]);
});

_t("TableColumnFlags", "Every TableColumnFlags constant is exposed as an integer", function(){
    ::checkConstants("TableColumnFlags", [
        "TableColumnFlags_None", "TableColumnFlags_DefaultHide", "TableColumnFlags_DefaultSort",
        "TableColumnFlags_WidthStretch", "TableColumnFlags_WidthFixed", "TableColumnFlags_NoResize",
        "TableColumnFlags_NoHide"
    ]);
});

_t("TableRowFlags", "Every TableRowFlags constant is exposed as an integer", function(){
    ::checkConstants("TableRowFlags", [
        "TableRowFlags_None", "TableRowFlags_Headers"
    ]);
});

_t("TabBarFlags", "Every TabBarFlags constant is exposed as an integer", function(){
    ::checkConstants("TabBarFlags", [
        "TabBarFlags_None", "TabBarFlags_Reorderable", "TabBarFlags_AutoSelectNewTabs",
        "TabBarFlags_TabListPopupButton", "TabBarFlags_NoCloseWithMiddleMouseButton", "TabBarFlags_NoTabListScrollingButtons",
        "TabBarFlags_NoTooltip", "TabBarFlags_FittingPolicyResizeDown", "TabBarFlags_FittingPolicyScroll"
    ]);
});

_t("TabItemFlags", "Every TabItemFlags constant is exposed as an integer", function(){
    ::checkConstants("TabItemFlags", [
        "TabItemFlags_None", "TabItemFlags_UnsavedDocument", "TabItemFlags_SetSelected",
        "TabItemFlags_NoCloseWithMiddleMouseButton", "TabItemFlags_NoPushId", "TabItemFlags_NoTooltip",
        "TabItemFlags_NoReorder", "TabItemFlags_Leading", "TabItemFlags_Trailing"
    ]);
});

_t("HoveredFlags", "Every HoveredFlags constant is exposed as an integer", function(){
    ::checkConstants("HoveredFlags", [
        "HoveredFlags_None", "HoveredFlags_ChildWindows", "HoveredFlags_RootWindow",
        "HoveredFlags_AnyWindow", "HoveredFlags_AllowWhenBlockedByPopup", "HoveredFlags_AllowWhenBlockedByActiveItem",
        "HoveredFlags_AllowWhenOverlapped", "HoveredFlags_AllowWhenDisabled", "HoveredFlags_RectOnly",
        "HoveredFlags_RootAndChildWindows", "HoveredFlags_ForTooltip", "HoveredFlags_DelayShort",
        "HoveredFlags_DelayNormal"
    ]);
});

_t("FocusedFlags", "Every FocusedFlags constant is exposed as an integer", function(){
    ::checkConstants("FocusedFlags", [
        "FocusedFlags_None", "FocusedFlags_ChildWindows", "FocusedFlags_RootWindow",
        "FocusedFlags_AnyWindow", "FocusedFlags_RootAndChildWindows"
    ]);
});

_t("PopupFlags", "Every PopupFlags constant is exposed as an integer", function(){
    ::checkConstants("PopupFlags", [
        "PopupFlags_None", "PopupFlags_NoReopen", "PopupFlags_NoOpenOverExistingPopup",
        "PopupFlags_AnyPopupId", "PopupFlags_AnyPopupLevel", "PopupFlags_AnyPopup"
    ]);
});

_t("SliderFlags", "Every SliderFlags constant is exposed as an integer", function(){
    ::checkConstants("SliderFlags", [
        "SliderFlags_None", "SliderFlags_Logarithmic", "SliderFlags_NoRoundToFormat",
        "SliderFlags_NoInput", "SliderFlags_AlwaysClamp"
    ]);
});

_t("ColorEditFlags", "Every ColorEditFlags constant is exposed as an integer", function(){
    ::checkConstants("ColorEditFlags", [
        "ColorEditFlags_None", "ColorEditFlags_NoAlpha", "ColorEditFlags_NoPicker",
        "ColorEditFlags_NoOptions", "ColorEditFlags_NoSmallPreview", "ColorEditFlags_NoInputs",
        "ColorEditFlags_NoTooltip", "ColorEditFlags_NoLabel", "ColorEditFlags_NoSidePreview",
        "ColorEditFlags_NoDragDrop", "ColorEditFlags_AlphaBar", "ColorEditFlags_AlphaPreviewHalf",
        "ColorEditFlags_DisplayRGB", "ColorEditFlags_DisplayHSV", "ColorEditFlags_DisplayHex",
        "ColorEditFlags_Float", "ColorEditFlags_PickerHueBar", "ColorEditFlags_PickerHueWheel"
    ]);
});

_t("MouseButton", "Every MouseButton constant is exposed as an integer", function(){
    ::checkConstants("MouseButton", [
        "MouseButton_Left", "MouseButton_Right", "MouseButton_Middle"
    ]);
});

_t("flagsCombine", "Flag constants are distinct bits that combine", function(){
    //Distinct single-bit flags must not collide.
    local a = _imgui.WindowFlags_NoTitleBar;
    local b = _imgui.WindowFlags_NoResize;
    _test.assertNotEqual(a, b);
    local combined = a | b;
    _test.assertEqual(a, combined & a);
    _test.assertEqual(b, combined & b);

    //None entries are zero.
    _test.assertEqual(0, _imgui.WindowFlags_None);
    _test.assertEqual(0, _imgui.TableFlags_None);
    _test.assertEqual(0, _imgui.Cond_None);
});
