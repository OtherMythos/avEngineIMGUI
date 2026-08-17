//Frame lifecycle, timing and the overlay workspace controls.

_t("getVersion", "Returns the imgui version string", function(){
    local v = _imgui.getVersion();
    _test.assertEqual("string", typeof v);
    _test.assertTrue(v.len() > 0);
});

_t("getTime", "Time advances and is a float", function(){
    local t = _imgui.getTime();
    _test.assertEqual("float", typeof t);
    _test.assertTrue(t > 0.0);
});

_t("getFrameCount", "Frame count is a positive integer", function(){
    local f = _imgui.getFrameCount();
    _test.assertEqual("integer", typeof f);
    _test.assertTrue(f > 0);
});

_t("getFramerate", "Framerate is a plausible float", function(){
    local fps = _imgui.getFramerate();
    _test.assertEqual("float", typeof fps);
    _test.assertTrue(fps > 0.0);
    _test.assertTrue(fps < 10000.0);
});

_t("getDeltaTime", "Delta time is positive and bounded by the manager's clamp", function(){
    local dt = _imgui.getDeltaTime();
    _test.assertEqual("float", typeof dt);
    _test.assertTrue(dt > 0.0);
    //ImguiManager clamps delta to 0.25s.
    _test.assertTrue(dt <= 0.25);
});

_t("getDisplaySize", "Display size is a two element array matching the window", function(){
    local size = _imgui.getDisplaySize();
    _test.assertEqual("array", typeof size);
    _test.assertEqual(2, size.len());
    _test.assertEqual("float", typeof size[0]);
    _test.assertEqual("float", typeof size[1]);
    _test.assertTrue(size[0] > 0.0);
    _test.assertTrue(size[1] > 0.0);

    //imgui's display size is the render target, i.e. the window's actual size.
    local actual = _window.getActualSize();
    ::_tNear(size[0], actual.x.tofloat(), 1.0);
    ::_tNear(size[1], actual.y.tofloat(), 1.0);
});

_t("wantCapture", "The three capture flags are booleans", function(){
    _test.assertEqual("bool", typeof _imgui.wantCaptureMouse());
    _test.assertEqual("bool", typeof _imgui.wantCaptureKeyboard());
    _test.assertEqual("bool", typeof _imgui.wantTextInput());
});

_t("isFirstUpdateOfFrame", "True at the start of a frame, false after the frame is begun", function(){
    //The harness runs one test per frame, and nothing has touched imgui yet
    //this frame, so a frame is still pending.
    _test.assertTrue(_imgui.isFirstUpdateOfFrame());
    //Any api call begins the frame; a second query in the same frame is false.
    _imgui.text("begin the frame");
    _test.assertFalse(_imgui.isFirstUpdateOfFrame());
});

_t("setRenderingEnabled", "Accepts both booleans and leaves the api usable", function(){
    _imgui.setRenderingEnabled(false);
    _imgui.begin("misc/renderingDisabled");
    _imgui.text("not rendered, but still valid");
    _imgui.end();
    _imgui.setRenderingEnabled(true);
});

_t("mouseInputEnabled", "Globally disables and restores imgui mouse interaction", function(){
    _test.assertTrue(_imgui.getMouseInputEnabled());
    _imgui.setMouseInputEnabled(false);
    _test.assertFalse(_imgui.getMouseInputEnabled());
    //Restore the process-wide flag for every test which follows.
    _imgui.setMouseInputEnabled(true);
    _test.assertTrue(_imgui.getMouseInputEnabled());
});

//The sizes at a scale of 1, so the scaled ones can be compared against them.
//Each test runs on its own frame, which is what a scale needs to take effect.
::gUnscaledFrameHeight <- 0.0;
::gUnscaledTextWidth <- 0.0;

_t("setGlobalScale", "The scale is reported straight back but only applies from the next frame", function(){
    _tNear(_imgui.getGlobalScale(), 1.0);

    ::gUnscaledFrameHeight = _imgui.getFrameHeight();
    ::gUnscaledTextWidth = _imgui.calcTextSize("scaled")[0];
    _test.assertTrue(::gUnscaledFrameHeight > 0.0);
    _test.assertTrue(::gUnscaledTextWidth > 0.0);

    _imgui.setGlobalScale(2.0);
    _tNear(_imgui.getGlobalScale(), 2.0);

    //This frame was begun before the scale was asked for, so it is still the
    //unscaled one being built.
    _tNear(_imgui.getFrameHeight(), ::gUnscaledFrameHeight);
});

_t("globalScaleApplied", "A scaled gui draws bigger widgets and text", function(){
    _imgui.begin("misc/scaled");
    _imgui.text("scaled");
    _imgui.end();

    //Not exactly twice: style sizes are truncated to whole pixels as they are
    //scaled, and the font is re-baked rather than stretched, so glyph advances
    //are the scaled font's own rather than doubled ones.
    _test.assertTrue(_imgui.getFrameHeight() > ::gUnscaledFrameHeight * 1.5);
    _test.assertTrue(_imgui.calcTextSize("scaled")[0] > ::gUnscaledTextWidth * 1.5);
});

_t("setGlobalScaleInvalid", "A scale of zero or less is ignored", function(){
    _imgui.setGlobalScale(0.0);
    _tNear(_imgui.getGlobalScale(), 2.0);
    _imgui.setGlobalScale(-1.0);
    _tNear(_imgui.getGlobalScale(), 2.0);

    //Put it back the way it was, for the tests which follow.
    _imgui.setGlobalScale(1.0);
});

_t("globalScaleRestored", "Scaling back down returns the sizes it started with", function(){
    _tNear(_imgui.getGlobalScale(), 1.0);
    _tNear(_imgui.getFrameHeight(), ::gUnscaledFrameHeight);
    _tNear(_imgui.calcTextSize("scaled")[0], ::gUnscaledTextWidth);
});

_t("autoOverlayEnabled", "The auto overlay flag round-trips", function(){
    _test.assertEqual("bool", typeof _imgui.getAutoOverlayEnabled());

    _imgui.setAutoOverlayEnabled(false);
    _test.assertFalse(_imgui.getAutoOverlayEnabled());
    _imgui.setAutoOverlayEnabled(true);
    _test.assertTrue(_imgui.getAutoOverlayEnabled());
});

_t("overlayWorkspace", "Creating and destroying the overlay reports whether it changed", function(){
    //Something has already been drawn by now, so the overlay exists and a
    //second create is a no-op.
    _test.assertFalse(_imgui.createOverlayWorkspace());

    _test.assertTrue(_imgui.destroyOverlayWorkspace());
    //Destroying twice reports no change, and disables automatic re-creation.
    _test.assertFalse(_imgui.destroyOverlayWorkspace());
    _test.assertFalse(_imgui.getAutoOverlayEnabled());

    //Put it back the way it was.
    _test.assertTrue(_imgui.createOverlayWorkspace());
    _imgui.setAutoOverlayEnabled(true);
});

_t("showDemoWindow", "The bundled demo and metrics windows submit without error", function(){
    _imgui.showDemoWindow();
    _imgui.showMetricsWindow();
});
