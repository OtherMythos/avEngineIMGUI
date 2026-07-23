//The id stack and the style stacks. Every push must be matched by a pop, and
//imgui checks the balance when the frame ends, so an imbalance here would fail
//the whole test rather than pass quietly.

_t("pushIdString", "String ids disambiguate identical labels", function(){
    _imgui.begin("style/idString");

    //Two buttons with the same label are distinct items under different ids.
    _imgui.pushId("first");
    _test.assertFalse(_imgui.button("duplicate"));
    _imgui.popId();

    _imgui.pushId("second");
    _test.assertFalse(_imgui.button("duplicate"));
    _imgui.popId();

    _imgui.end();
});

_t("pushIdInteger", "Integer ids are accepted, as in a loop", function(){
    _imgui.begin("style/idInteger");
    for(local i = 0; i < 5; i++){
        _imgui.pushId(i);
        _imgui.button("row");
        _imgui.popId();
    }
    _imgui.end();
});

_t("pushIdNested", "Id pushes nest", function(){
    _imgui.begin("style/idNested");
    _imgui.pushId("outer");
    _imgui.pushId("inner");
    _imgui.button("nested");
    _imgui.popId();
    _imgui.popId();
    _imgui.end();
});

_t("pushStyleColor", "Colours are pushed and popped, singly and in bulk", function(){
    _imgui.begin("style/colours");

    _imgui.pushStyleColor(_imgui.Col_Text, 1.0, 0.0, 0.0, 1.0);
    _imgui.text("red text");
    _imgui.popStyleColor();

    //Several at once, popped with a count.
    _imgui.pushStyleColor(_imgui.Col_Button, 0.2, 0.4, 0.8, 1.0);
    _imgui.pushStyleColor(_imgui.Col_ButtonHovered, 0.3, 0.5, 0.9, 1.0);
    _imgui.pushStyleColor(_imgui.Col_ButtonActive, 0.1, 0.3, 0.7, 1.0);
    _imgui.button("styled");
    _imgui.popStyleColor(3);

    _imgui.end();
});

_t("styleColourConstants", "A representative spread of colour constants is accepted", function(){
    _imgui.begin("style/colourConstants");
    local cols = [
        _imgui.Col_Text, _imgui.Col_TextDisabled, _imgui.Col_WindowBg, _imgui.Col_ChildBg,
        _imgui.Col_PopupBg, _imgui.Col_Border, _imgui.Col_FrameBg, _imgui.Col_TitleBg,
        _imgui.Col_MenuBarBg, _imgui.Col_ScrollbarBg, _imgui.Col_CheckMark, _imgui.Col_SliderGrab,
        _imgui.Col_Header, _imgui.Col_Separator, _imgui.Col_PlotLines, _imgui.Col_PlotHistogram,
        _imgui.Col_TableHeaderBg, _imgui.Col_TableRowBg, _imgui.Col_TableRowBgAlt
    ];
    foreach(c in cols){
        _imgui.pushStyleColor(c, 0.5, 0.5, 0.5, 1.0);
    }
    _imgui.text("many colours pushed");
    _imgui.popStyleColor(cols.len());
    _imgui.end();
});

_t("pushStyleVarFloat", "Single component style vars are pushed and popped", function(){
    _imgui.begin("style/varFloat");
    _imgui.pushStyleVar(_imgui.StyleVar_Alpha, 0.5);
    _imgui.text("half alpha");
    _imgui.popStyleVar();

    _imgui.pushStyleVar(_imgui.StyleVar_FrameRounding, 6.0);
    _imgui.pushStyleVar(_imgui.StyleVar_ChildRounding, 4.0);
    _imgui.button("rounded");
    _imgui.popStyleVar(2);
    _imgui.end();
});

_t("pushStyleVarVec2", "Two component style vars take an x and a y", function(){
    _imgui.begin("style/varVec2");
    _imgui.pushStyleVar(_imgui.StyleVar_ItemSpacing, 20.0, 12.0);
    _imgui.text("spaced a");
    _imgui.text("spaced b");
    _imgui.popStyleVar();

    _imgui.pushStyleVar(_imgui.StyleVar_WindowPadding, 16.0, 16.0);
    _imgui.pushStyleVar(_imgui.StyleVar_FramePadding, 8.0, 4.0);
    _imgui.button("padded");
    _imgui.popStyleVar(2);
    _imgui.end();
});

_t("styleVarAffectsLayout", "A pushed spacing actually changes layout", function(){
    _imgui.begin("style/varEffect");

    _imgui.text("reference");
    local tight = _imgui.getCursorPosY();
    _imgui.text("next");
    local tightStep = _imgui.getCursorPosY() - tight;

    _imgui.pushStyleVar(_imgui.StyleVar_ItemSpacing, 0.0, 40.0);
    local wide = _imgui.getCursorPosY();
    _imgui.text("spaced");
    local wideStep = _imgui.getCursorPosY() - wide;
    _imgui.popStyleVar();

    _imgui.end();

    _test.assertTrue(wideStep > tightStep);
});
