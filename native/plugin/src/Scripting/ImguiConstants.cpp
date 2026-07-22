#include "ImguiNamespace.h"

#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "imgui.h"

namespace AVImgui{

    namespace {
        struct ConstantEntry{
            const char* name;
            SQInteger value;
        };

        //The exposed names drop the 'ImGui' prefix, so ImGuiWindowFlags_NoResize
        //becomes _imgui.WindowFlags_NoResize.
        static const ConstantEntry imguiConstants[] = {
            //ImGuiWindowFlags
            {"WindowFlags_None", ImGuiWindowFlags_None},
            {"WindowFlags_NoTitleBar", ImGuiWindowFlags_NoTitleBar},
            {"WindowFlags_NoResize", ImGuiWindowFlags_NoResize},
            {"WindowFlags_NoMove", ImGuiWindowFlags_NoMove},
            {"WindowFlags_NoScrollbar", ImGuiWindowFlags_NoScrollbar},
            {"WindowFlags_NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse},
            {"WindowFlags_NoCollapse", ImGuiWindowFlags_NoCollapse},
            {"WindowFlags_AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize},
            {"WindowFlags_NoBackground", ImGuiWindowFlags_NoBackground},
            {"WindowFlags_NoSavedSettings", ImGuiWindowFlags_NoSavedSettings},
            {"WindowFlags_NoMouseInputs", ImGuiWindowFlags_NoMouseInputs},
            {"WindowFlags_MenuBar", ImGuiWindowFlags_MenuBar},
            {"WindowFlags_HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar},
            {"WindowFlags_NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing},
            {"WindowFlags_NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus},
            {"WindowFlags_AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar},
            {"WindowFlags_AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar},
            {"WindowFlags_NoNavInputs", ImGuiWindowFlags_NoNavInputs},
            {"WindowFlags_NoNavFocus", ImGuiWindowFlags_NoNavFocus},
            {"WindowFlags_UnsavedDocument", ImGuiWindowFlags_UnsavedDocument},
            {"WindowFlags_NoNav", ImGuiWindowFlags_NoNav},
            {"WindowFlags_NoDecoration", ImGuiWindowFlags_NoDecoration},
            {"WindowFlags_NoInputs", ImGuiWindowFlags_NoInputs},

            //ImGuiChildFlags
            {"ChildFlags_None", ImGuiChildFlags_None},
            {"ChildFlags_Borders", ImGuiChildFlags_Borders},
            {"ChildFlags_AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding},
            {"ChildFlags_ResizeX", ImGuiChildFlags_ResizeX},
            {"ChildFlags_ResizeY", ImGuiChildFlags_ResizeY},
            {"ChildFlags_AutoResizeX", ImGuiChildFlags_AutoResizeX},
            {"ChildFlags_AutoResizeY", ImGuiChildFlags_AutoResizeY},
            {"ChildFlags_AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize},
            {"ChildFlags_FrameStyle", ImGuiChildFlags_FrameStyle},

            //ImGuiCond
            {"Cond_None", ImGuiCond_None},
            {"Cond_Always", ImGuiCond_Always},
            {"Cond_Once", ImGuiCond_Once},
            {"Cond_FirstUseEver", ImGuiCond_FirstUseEver},
            {"Cond_Appearing", ImGuiCond_Appearing},

            //ImGuiDir
            {"Dir_Left", ImGuiDir_Left},
            {"Dir_Right", ImGuiDir_Right},
            {"Dir_Up", ImGuiDir_Up},
            {"Dir_Down", ImGuiDir_Down},

            //ImGuiCol
            {"Col_Text", ImGuiCol_Text},
            {"Col_TextDisabled", ImGuiCol_TextDisabled},
            {"Col_WindowBg", ImGuiCol_WindowBg},
            {"Col_ChildBg", ImGuiCol_ChildBg},
            {"Col_PopupBg", ImGuiCol_PopupBg},
            {"Col_Border", ImGuiCol_Border},
            {"Col_FrameBg", ImGuiCol_FrameBg},
            {"Col_FrameBgHovered", ImGuiCol_FrameBgHovered},
            {"Col_FrameBgActive", ImGuiCol_FrameBgActive},
            {"Col_TitleBg", ImGuiCol_TitleBg},
            {"Col_TitleBgActive", ImGuiCol_TitleBgActive},
            {"Col_TitleBgCollapsed", ImGuiCol_TitleBgCollapsed},
            {"Col_MenuBarBg", ImGuiCol_MenuBarBg},
            {"Col_ScrollbarBg", ImGuiCol_ScrollbarBg},
            {"Col_CheckMark", ImGuiCol_CheckMark},
            {"Col_SliderGrab", ImGuiCol_SliderGrab},
            {"Col_SliderGrabActive", ImGuiCol_SliderGrabActive},
            {"Col_Button", ImGuiCol_Button},
            {"Col_ButtonHovered", ImGuiCol_ButtonHovered},
            {"Col_ButtonActive", ImGuiCol_ButtonActive},
            {"Col_Header", ImGuiCol_Header},
            {"Col_HeaderHovered", ImGuiCol_HeaderHovered},
            {"Col_HeaderActive", ImGuiCol_HeaderActive},
            {"Col_Separator", ImGuiCol_Separator},
            {"Col_PlotLines", ImGuiCol_PlotLines},
            {"Col_PlotHistogram", ImGuiCol_PlotHistogram},
            {"Col_TableHeaderBg", ImGuiCol_TableHeaderBg},
            {"Col_TableRowBg", ImGuiCol_TableRowBg},
            {"Col_TableRowBgAlt", ImGuiCol_TableRowBgAlt},

            //ImGuiStyleVar
            {"StyleVar_Alpha", ImGuiStyleVar_Alpha},
            {"StyleVar_DisabledAlpha", ImGuiStyleVar_DisabledAlpha},
            {"StyleVar_WindowPadding", ImGuiStyleVar_WindowPadding},
            {"StyleVar_WindowRounding", ImGuiStyleVar_WindowRounding},
            {"StyleVar_WindowBorderSize", ImGuiStyleVar_WindowBorderSize},
            {"StyleVar_WindowMinSize", ImGuiStyleVar_WindowMinSize},
            {"StyleVar_WindowTitleAlign", ImGuiStyleVar_WindowTitleAlign},
            {"StyleVar_ChildRounding", ImGuiStyleVar_ChildRounding},
            {"StyleVar_ChildBorderSize", ImGuiStyleVar_ChildBorderSize},
            {"StyleVar_PopupRounding", ImGuiStyleVar_PopupRounding},
            {"StyleVar_PopupBorderSize", ImGuiStyleVar_PopupBorderSize},
            {"StyleVar_FramePadding", ImGuiStyleVar_FramePadding},
            {"StyleVar_FrameRounding", ImGuiStyleVar_FrameRounding},
            {"StyleVar_FrameBorderSize", ImGuiStyleVar_FrameBorderSize},
            {"StyleVar_ItemSpacing", ImGuiStyleVar_ItemSpacing},
            {"StyleVar_ItemInnerSpacing", ImGuiStyleVar_ItemInnerSpacing},
            {"StyleVar_IndentSpacing", ImGuiStyleVar_IndentSpacing},
            {"StyleVar_CellPadding", ImGuiStyleVar_CellPadding},
            {"StyleVar_ScrollbarSize", ImGuiStyleVar_ScrollbarSize},
            {"StyleVar_ScrollbarRounding", ImGuiStyleVar_ScrollbarRounding},
            {"StyleVar_GrabMinSize", ImGuiStyleVar_GrabMinSize},
            {"StyleVar_GrabRounding", ImGuiStyleVar_GrabRounding},
            {"StyleVar_TabRounding", ImGuiStyleVar_TabRounding},
            {"StyleVar_ButtonTextAlign", ImGuiStyleVar_ButtonTextAlign},
            {"StyleVar_SelectableTextAlign", ImGuiStyleVar_SelectableTextAlign},
            {"StyleVar_SeparatorTextBorderSize", ImGuiStyleVar_SeparatorTextBorderSize},
            {"StyleVar_SeparatorTextAlign", ImGuiStyleVar_SeparatorTextAlign},
            {"StyleVar_SeparatorTextPadding", ImGuiStyleVar_SeparatorTextPadding},

            //ImGuiTreeNodeFlags
            {"TreeNodeFlags_None", ImGuiTreeNodeFlags_None},
            {"TreeNodeFlags_Selected", ImGuiTreeNodeFlags_Selected},
            {"TreeNodeFlags_Framed", ImGuiTreeNodeFlags_Framed},
            {"TreeNodeFlags_AllowOverlap", ImGuiTreeNodeFlags_AllowOverlap},
            {"TreeNodeFlags_NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen},
            {"TreeNodeFlags_DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen},
            {"TreeNodeFlags_OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick},
            {"TreeNodeFlags_OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow},
            {"TreeNodeFlags_Leaf", ImGuiTreeNodeFlags_Leaf},
            {"TreeNodeFlags_Bullet", ImGuiTreeNodeFlags_Bullet},
            {"TreeNodeFlags_FramePadding", ImGuiTreeNodeFlags_FramePadding},
            {"TreeNodeFlags_SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth},
            {"TreeNodeFlags_SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth},
            {"TreeNodeFlags_CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader},

            //ImGuiInputTextFlags
            {"InputTextFlags_None", ImGuiInputTextFlags_None},
            {"InputTextFlags_CharsDecimal", ImGuiInputTextFlags_CharsDecimal},
            {"InputTextFlags_CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal},
            {"InputTextFlags_CharsScientific", ImGuiInputTextFlags_CharsScientific},
            {"InputTextFlags_CharsUppercase", ImGuiInputTextFlags_CharsUppercase},
            {"InputTextFlags_CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank},
            {"InputTextFlags_AllowTabInput", ImGuiInputTextFlags_AllowTabInput},
            {"InputTextFlags_EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue},
            {"InputTextFlags_EscapeClearsAll", ImGuiInputTextFlags_EscapeClearsAll},
            {"InputTextFlags_CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine},
            {"InputTextFlags_ReadOnly", ImGuiInputTextFlags_ReadOnly},
            {"InputTextFlags_Password", ImGuiInputTextFlags_Password},
            {"InputTextFlags_AlwaysOverwrite", ImGuiInputTextFlags_AlwaysOverwrite},
            {"InputTextFlags_AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll},
            {"InputTextFlags_NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll},
            {"InputTextFlags_NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo},

            //ImGuiSelectableFlags
            {"SelectableFlags_None", ImGuiSelectableFlags_None},
            {"SelectableFlags_NoAutoClosePopups", ImGuiSelectableFlags_NoAutoClosePopups},
            {"SelectableFlags_SpanAllColumns", ImGuiSelectableFlags_SpanAllColumns},
            {"SelectableFlags_AllowDoubleClick", ImGuiSelectableFlags_AllowDoubleClick},
            {"SelectableFlags_Disabled", ImGuiSelectableFlags_Disabled},
            {"SelectableFlags_AllowOverlap", ImGuiSelectableFlags_AllowOverlap},

            //ImGuiComboFlags
            {"ComboFlags_None", ImGuiComboFlags_None},
            {"ComboFlags_PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft},
            {"ComboFlags_HeightSmall", ImGuiComboFlags_HeightSmall},
            {"ComboFlags_HeightRegular", ImGuiComboFlags_HeightRegular},
            {"ComboFlags_HeightLarge", ImGuiComboFlags_HeightLarge},
            {"ComboFlags_HeightLargest", ImGuiComboFlags_HeightLargest},
            {"ComboFlags_NoArrowButton", ImGuiComboFlags_NoArrowButton},
            {"ComboFlags_NoPreview", ImGuiComboFlags_NoPreview},
            {"ComboFlags_WidthFitPreview", ImGuiComboFlags_WidthFitPreview},

            //ImGuiTableFlags
            {"TableFlags_None", ImGuiTableFlags_None},
            {"TableFlags_Resizable", ImGuiTableFlags_Resizable},
            {"TableFlags_Reorderable", ImGuiTableFlags_Reorderable},
            {"TableFlags_Hideable", ImGuiTableFlags_Hideable},
            {"TableFlags_Sortable", ImGuiTableFlags_Sortable},
            {"TableFlags_NoSavedSettings", ImGuiTableFlags_NoSavedSettings},
            {"TableFlags_ContextMenuInBody", ImGuiTableFlags_ContextMenuInBody},
            {"TableFlags_RowBg", ImGuiTableFlags_RowBg},
            {"TableFlags_BordersInnerH", ImGuiTableFlags_BordersInnerH},
            {"TableFlags_BordersOuterH", ImGuiTableFlags_BordersOuterH},
            {"TableFlags_BordersInnerV", ImGuiTableFlags_BordersInnerV},
            {"TableFlags_BordersOuterV", ImGuiTableFlags_BordersOuterV},
            {"TableFlags_BordersH", ImGuiTableFlags_BordersH},
            {"TableFlags_BordersV", ImGuiTableFlags_BordersV},
            {"TableFlags_BordersInner", ImGuiTableFlags_BordersInner},
            {"TableFlags_BordersOuter", ImGuiTableFlags_BordersOuter},
            {"TableFlags_Borders", ImGuiTableFlags_Borders},
            {"TableFlags_SizingFixedFit", ImGuiTableFlags_SizingFixedFit},
            {"TableFlags_SizingFixedSame", ImGuiTableFlags_SizingFixedSame},
            {"TableFlags_SizingStretchProp", ImGuiTableFlags_SizingStretchProp},
            {"TableFlags_SizingStretchSame", ImGuiTableFlags_SizingStretchSame},
            {"TableFlags_NoHostExtendX", ImGuiTableFlags_NoHostExtendX},
            {"TableFlags_NoHostExtendY", ImGuiTableFlags_NoHostExtendY},
            {"TableFlags_ScrollX", ImGuiTableFlags_ScrollX},
            {"TableFlags_ScrollY", ImGuiTableFlags_ScrollY},

            //ImGuiTableColumnFlags
            {"TableColumnFlags_None", ImGuiTableColumnFlags_None},
            {"TableColumnFlags_DefaultHide", ImGuiTableColumnFlags_DefaultHide},
            {"TableColumnFlags_DefaultSort", ImGuiTableColumnFlags_DefaultSort},
            {"TableColumnFlags_WidthStretch", ImGuiTableColumnFlags_WidthStretch},
            {"TableColumnFlags_WidthFixed", ImGuiTableColumnFlags_WidthFixed},
            {"TableColumnFlags_NoResize", ImGuiTableColumnFlags_NoResize},
            {"TableColumnFlags_NoHide", ImGuiTableColumnFlags_NoHide},

            //ImGuiTableRowFlags
            {"TableRowFlags_None", ImGuiTableRowFlags_None},
            {"TableRowFlags_Headers", ImGuiTableRowFlags_Headers},

            //ImGuiTabBarFlags
            {"TabBarFlags_None", ImGuiTabBarFlags_None},
            {"TabBarFlags_Reorderable", ImGuiTabBarFlags_Reorderable},
            {"TabBarFlags_AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs},
            {"TabBarFlags_TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton},
            {"TabBarFlags_NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton},
            {"TabBarFlags_NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons},
            {"TabBarFlags_NoTooltip", ImGuiTabBarFlags_NoTooltip},
            {"TabBarFlags_FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown},
            {"TabBarFlags_FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll},

            //ImGuiTabItemFlags
            {"TabItemFlags_None", ImGuiTabItemFlags_None},
            {"TabItemFlags_UnsavedDocument", ImGuiTabItemFlags_UnsavedDocument},
            {"TabItemFlags_SetSelected", ImGuiTabItemFlags_SetSelected},
            {"TabItemFlags_NoCloseWithMiddleMouseButton", ImGuiTabItemFlags_NoCloseWithMiddleMouseButton},
            {"TabItemFlags_NoPushId", ImGuiTabItemFlags_NoPushId},
            {"TabItemFlags_NoTooltip", ImGuiTabItemFlags_NoTooltip},
            {"TabItemFlags_NoReorder", ImGuiTabItemFlags_NoReorder},
            {"TabItemFlags_Leading", ImGuiTabItemFlags_Leading},
            {"TabItemFlags_Trailing", ImGuiTabItemFlags_Trailing},

            //ImGuiHoveredFlags
            {"HoveredFlags_None", ImGuiHoveredFlags_None},
            {"HoveredFlags_ChildWindows", ImGuiHoveredFlags_ChildWindows},
            {"HoveredFlags_RootWindow", ImGuiHoveredFlags_RootWindow},
            {"HoveredFlags_AnyWindow", ImGuiHoveredFlags_AnyWindow},
            {"HoveredFlags_AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup},
            {"HoveredFlags_AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem},
            {"HoveredFlags_AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped},
            {"HoveredFlags_AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled},
            {"HoveredFlags_RectOnly", ImGuiHoveredFlags_RectOnly},
            {"HoveredFlags_RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows},
            {"HoveredFlags_ForTooltip", ImGuiHoveredFlags_ForTooltip},
            {"HoveredFlags_DelayShort", ImGuiHoveredFlags_DelayShort},
            {"HoveredFlags_DelayNormal", ImGuiHoveredFlags_DelayNormal},

            //ImGuiFocusedFlags
            {"FocusedFlags_None", ImGuiFocusedFlags_None},
            {"FocusedFlags_ChildWindows", ImGuiFocusedFlags_ChildWindows},
            {"FocusedFlags_RootWindow", ImGuiFocusedFlags_RootWindow},
            {"FocusedFlags_AnyWindow", ImGuiFocusedFlags_AnyWindow},
            {"FocusedFlags_RootAndChildWindows", ImGuiFocusedFlags_RootAndChildWindows},

            //ImGuiPopupFlags
            {"PopupFlags_None", ImGuiPopupFlags_None},
            {"PopupFlags_NoReopen", ImGuiPopupFlags_NoReopen},
            {"PopupFlags_NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup},
            {"PopupFlags_AnyPopupId", ImGuiPopupFlags_AnyPopupId},
            {"PopupFlags_AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel},
            {"PopupFlags_AnyPopup", ImGuiPopupFlags_AnyPopup},

            //ImGuiSliderFlags
            {"SliderFlags_None", ImGuiSliderFlags_None},
            {"SliderFlags_Logarithmic", ImGuiSliderFlags_Logarithmic},
            {"SliderFlags_NoRoundToFormat", ImGuiSliderFlags_NoRoundToFormat},
            {"SliderFlags_NoInput", ImGuiSliderFlags_NoInput},
            {"SliderFlags_AlwaysClamp", ImGuiSliderFlags_AlwaysClamp},

            //ImGuiColorEditFlags
            {"ColorEditFlags_None", ImGuiColorEditFlags_None},
            {"ColorEditFlags_NoAlpha", ImGuiColorEditFlags_NoAlpha},
            {"ColorEditFlags_NoPicker", ImGuiColorEditFlags_NoPicker},
            {"ColorEditFlags_NoOptions", ImGuiColorEditFlags_NoOptions},
            {"ColorEditFlags_NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview},
            {"ColorEditFlags_NoInputs", ImGuiColorEditFlags_NoInputs},
            {"ColorEditFlags_NoTooltip", ImGuiColorEditFlags_NoTooltip},
            {"ColorEditFlags_NoLabel", ImGuiColorEditFlags_NoLabel},
            {"ColorEditFlags_NoSidePreview", ImGuiColorEditFlags_NoSidePreview},
            {"ColorEditFlags_NoDragDrop", ImGuiColorEditFlags_NoDragDrop},
            {"ColorEditFlags_AlphaBar", ImGuiColorEditFlags_AlphaBar},
            {"ColorEditFlags_AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf},
            {"ColorEditFlags_DisplayRGB", ImGuiColorEditFlags_DisplayRGB},
            {"ColorEditFlags_DisplayHSV", ImGuiColorEditFlags_DisplayHSV},
            {"ColorEditFlags_DisplayHex", ImGuiColorEditFlags_DisplayHex},
            {"ColorEditFlags_Float", ImGuiColorEditFlags_Float},
            {"ColorEditFlags_PickerHueBar", ImGuiColorEditFlags_PickerHueBar},
            {"ColorEditFlags_PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel},

            //ImGuiMouseButton
            {"MouseButton_Left", ImGuiMouseButton_Left},
            {"MouseButton_Right", ImGuiMouseButton_Right},
            {"MouseButton_Middle", ImGuiMouseButton_Middle},
        };
    }

    void ImguiNamespace::_setupConstants(HSQUIRRELVM vm){
        for(const ConstantEntry& entry : imguiConstants){
            AV::ScriptUtils::declareConstant(vm, entry.name, entry.value);
        }
    }
}
