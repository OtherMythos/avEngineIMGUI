//imgui's own mouse state. This is the source a tool should read inside an imgui
//window, since the engine's input layer swallows anything imgui captures.
//Nothing is clicked here, so the pressed states must all report false.

_t("mouseQueryTypes", "Every mouse query returns the right type", function(){
    _test.assertEqual("bool", typeof _imgui.isMouseDown());
    _test.assertEqual("bool", typeof _imgui.isMouseClicked());
    _test.assertEqual("bool", typeof _imgui.isMouseReleased());
    _test.assertEqual("bool", typeof _imgui.isMouseDoubleClicked());
    _test.assertEqual("bool", typeof _imgui.isAnyMouseDown());
    _test.assertEqual("bool", typeof _imgui.isMouseDragging());

    _test.assertEqual("array", typeof _imgui.getMousePos());
    _test.assertEqual("array", typeof _imgui.getMouseDragDelta());
    _test.assertEqual("array", typeof _imgui.getMouseWheel());
});

_t("untouchedMouseIsUp", "An untouched mouse reports no press", function(){
    _test.assertFalse(_imgui.isMouseDown());
    _test.assertFalse(_imgui.isMouseClicked());
    _test.assertFalse(_imgui.isMouseDoubleClicked());
    _test.assertFalse(_imgui.isAnyMouseDown());
    _test.assertFalse(_imgui.isMouseDragging());
});

_t("mouseButtonArgument", "Each button can be queried separately", function(){
    foreach(button in [_imgui.MouseButton_Left, _imgui.MouseButton_Right, _imgui.MouseButton_Middle]){
        _test.assertEqual("bool", typeof _imgui.isMouseDown(button));
        _test.assertEqual("bool", typeof _imgui.isMouseClicked(button));
        _test.assertEqual("bool", typeof _imgui.isMouseReleased(button));
        _test.assertEqual("bool", typeof _imgui.isMouseDragging(button));
        _test.assertEqual("array", typeof _imgui.getMouseDragDelta(button));
    }
});

_t("mouseVectorsHaveTwoComponents", "Positions and deltas come back as pairs", function(){
    local pos = _imgui.getMousePos();
    _test.assertEqual(2, pos.len());
    _test.assertEqual("float", typeof pos[0]);
    _test.assertEqual("float", typeof pos[1]);

    local delta = _imgui.getMouseDragDelta();
    _test.assertEqual(2, delta.len());

    //Vertical first, then horizontal.
    local wheel = _imgui.getMouseWheel();
    _test.assertEqual(2, wheel.len());
    _test.assertEqual("float", typeof wheel[0]);
    _test.assertEqual("float", typeof wheel[1]);
});

_t("undraggedDeltaIsZero", "An undragged mouse has no drag delta", function(){
    local delta = _imgui.getMouseDragDelta();
    _test.assertEqual(0.0, delta[0]);
    _test.assertEqual(0.0, delta[1]);
});

_t("mouseOptionalArguments", "The repeat and threshold arguments are accepted", function(){
    _test.assertEqual("bool", typeof _imgui.isMouseClicked(_imgui.MouseButton_Left, true));
    //-1 asks for imgui's own configured drag threshold.
    _test.assertEqual("bool", typeof _imgui.isMouseDragging(_imgui.MouseButton_Left, -1.0));
    _test.assertEqual("array", typeof _imgui.getMouseDragDelta(_imgui.MouseButton_Left, 20.0));
});

_t("resetMouseDragDelta", "The drag delta can be reset", function(){
    _imgui.resetMouseDragDelta();
    _imgui.resetMouseDragDelta(_imgui.MouseButton_Right);

    local delta = _imgui.getMouseDragDelta();
    _test.assertEqual(0.0, delta[0]);
    _test.assertEqual(0.0, delta[1]);
});

_t("popupMousePos", "The popup open position is a pair", function(){
    local pos = _imgui.getMousePosOnOpeningCurrentPopup();
    _test.assertEqual(2, pos.len());
});
