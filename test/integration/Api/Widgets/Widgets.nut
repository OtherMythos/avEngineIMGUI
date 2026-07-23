//Buttons and the other simple widgets. Nothing is clicked, so every "was it
//activated" result must come back false, and every state must round-trip.

_t("button", "Buttons return false when untouched, with and without a size", function(){
    _imgui.begin("widgets/button");

    local pressed = _imgui.button("button");
    _test.assertEqual("bool", typeof pressed);
    _test.assertFalse(pressed);

    //Explicit size.
    _test.assertFalse(_imgui.button("sizedButton", 120, 30));
    //Width only; height defaults.
    _test.assertFalse(_imgui.button("widthOnly", 80));

    _imgui.end();
});

_t("smallButton", "Small buttons behave like buttons", function(){
    _imgui.begin("widgets/smallButton");
    local pressed = _imgui.smallButton("small");
    _test.assertEqual("bool", typeof pressed);
    _test.assertFalse(pressed);
    _imgui.end();
});

_t("invisibleButton", "Invisible buttons need an id and a size", function(){
    _imgui.begin("widgets/invisibleButton");
    local pressed = _imgui.invisibleButton("invisible", 60, 20);
    _test.assertEqual("bool", typeof pressed);
    _test.assertFalse(pressed);
    //With flags.
    _test.assertFalse(_imgui.invisibleButton("invisibleFlags", 60, 20, 0));
    _imgui.end();
});

_t("arrowButton", "Every direction constant is accepted", function(){
    _imgui.begin("widgets/arrowButton");
    foreach(dir in [_imgui.Dir_Left, _imgui.Dir_Right, _imgui.Dir_Up, _imgui.Dir_Down]){
        local pressed = _imgui.arrowButton("arrow" + dir, dir);
        _test.assertEqual("bool", typeof pressed);
        _test.assertFalse(pressed);
    }
    _imgui.end();
});

_t("checkbox", "The checked state round-trips when untouched", function(){
    _imgui.begin("widgets/checkbox");

    local checkedOn = _imgui.checkbox("checkedOn", true);
    _test.assertEqual("bool", typeof checkedOn);
    _test.assertTrue(checkedOn);

    local checkedOff = _imgui.checkbox("checkedOff", false);
    _test.assertFalse(checkedOff);

    _imgui.end();
});

_t("radioButton", "Radio buttons report activation, not their state", function(){
    _imgui.begin("widgets/radio");
    //The argument is whether it is drawn as selected; the return is whether it
    //was just pressed, which it wasn't.
    _test.assertFalse(_imgui.radioButton("radioActive", true));
    _test.assertFalse(_imgui.radioButton("radioInactive", false));
    _imgui.end();
});

_t("selectable", "Selectable accepts its optional selected state, flags and size", function(){
    _imgui.begin("widgets/selectable");

    local clicked = _imgui.selectable("selectableDefaults");
    _test.assertEqual("bool", typeof clicked);
    _test.assertFalse(clicked);

    _test.assertFalse(_imgui.selectable("selectableSelected", true));
    _test.assertFalse(_imgui.selectable("selectableFlags", false, _imgui.SelectableFlags_Disabled));
    _test.assertFalse(_imgui.selectable("selectableSized", false, 0, 150, 20));

    _imgui.end();
});

_t("progressBar", "Progress accepts a fraction, an optional size and an overlay", function(){
    _imgui.begin("widgets/progress");
    _imgui.progressBar(0.0);
    _imgui.progressBar(0.5);
    _imgui.progressBar(1.0);
    _imgui.progressBar(0.5, 200, 20);
    _imgui.progressBar(0.5, 200, 20, "50%");
    //The overlay is optional and documented as accepting null.
    _imgui.progressBar(0.5, 200, 20, null);
    _imgui.end();
});

_t("bullet", "The bullet marker is drawn", function(){
    _imgui.begin("widgets/bullet");
    _imgui.bullet();
    _imgui.text("after a bullet");
    _imgui.end();
});
