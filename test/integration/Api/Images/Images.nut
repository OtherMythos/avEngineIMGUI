//Drawing an engine texture into an imgui window. The texture here is a render
//target created from script, which is the case the api exists for: an editor
//rendering its scene to a texture and showing it in a docked window.
//
//Nothing checks pixels — there is no way to read the gui back — so these assert
//the two things which are observable from script: that the call lays out the
//space it was asked for, and that bad input is an error rather than a crash.

local imageTexture = null;

_t("createTexture", "A render texture can be made for the image tests", function(){
    imageTexture = _graphics.createTexture("imgui/test/imageTexture");
    imageTexture.setPixelFormat(_PFG_RGBA8_UNORM);
    imageTexture.setResolution(64, 64);
    imageTexture.scheduleTransitionTo(_GPU_RESIDENCY_RESIDENT);

    _test.assertEqual(64, imageTexture.getWidth());
    _test.assertEqual(64, imageTexture.getHeight());
});

_t("image", "An image takes up the size it was given", function(){
    _imgui.begin("images/image");

    //Residency is asynchronous, so the texture may or may not be on the gpu by
    //now. Either way the layout is the same: a texture which is not yet
    //resident occupies its space and appears once it arrives.
    local before = _imgui.getCursorPosY();
    _imgui.image(imageTexture, 128, 64);
    local after = _imgui.getCursorPosY();

    _test.assertTrue(after - before >= 64);

    _imgui.end();
});

_t("imageUvs", "Uvs are optional and a sub-rect can be drawn", function(){
    _imgui.begin("images/imageUvs");

    //The whole texture, spelled out.
    _imgui.image(imageTexture, 32, 32, 0, 0, 1, 1);
    //The top left quarter.
    _imgui.image(imageTexture, 32, 32, 0, 0, 0.5, 0.5);
    //Flipped vertically, which is how a render target is corrected on an api
    //whose texture origin disagrees with the one it was rendered on.
    _imgui.image(imageTexture, 32, 32, 0, 1, 1, 0);

    _imgui.end();
});

_t("imageErrors", "Bad arguments are errors rather than crashes", function(){
    _imgui.begin("images/imageErrors");

    ::_tThrows("no arguments", function(){
        _imgui.image();
    });
    ::_tThrows("a string where a texture is expected", function(){
        _imgui.image("notATexture", 32, 32);
    });
    ::_tThrows("a texture with no size", function(){
        _imgui.image(imageTexture);
    });
    //A camera is a user data, but not a texture one, so the type tag check is
    //what has to reject it.
    ::_tThrows("the wrong kind of user data", function(){
        _imgui.image(_camera.getCamera(), 32, 32);
    });

    _imgui.end();
});

_t("imageDestroyedTexture", "A destroyed texture is an error, not a dangling bind", function(){
    _graphics.destroyTexture(imageTexture);

    _imgui.begin("images/imageDestroyedTexture");
    ::_tThrows("a texture which has been destroyed", function(){
        _imgui.image(imageTexture, 32, 32);
    });
    _imgui.end();

    imageTexture = null;
});
