//Bad calls must raise a squirrel error rather than take the engine down with
//them. A crash here fails the whole test case, which is the point: this is the
//difference between a typo in a debug overlay being an error in the log and it
//killing the game.

_t("wrongArgumentTypes", "Passing the wrong type is an error", function(){
    ::_tThrows("number where a string is expected", function(){
        _imgui.text(5);
    });
    ::_tThrows("table where a string is expected", function(){
        _imgui.begin({});
    });
    ::_tThrows("array where a string is expected", function(){
        _imgui.button([1, 2]);
    });
    ::_tThrows("string where a number is expected", function(){
        _imgui.setNextWindowBgAlpha("half");
    });
    ::_tThrows("string where a bool is expected", function(){
        _imgui.setRenderingEnabled("yes");
    });
    ::_tThrows("string where an array is expected", function(){
        _imgui.plotLines("##bad", "not an array");
    });
    ::_tThrows("string where an integer is expected", function(){
        _imgui.tableSetColumnIndex("first");
    });
});

_t("missingArguments", "Omitting a required argument is an error", function(){
    ::_tThrows("text with no arguments", function(){
        _imgui.text();
    });
    ::_tThrows("begin with no arguments", function(){
        _imgui.begin();
    });
    ::_tThrows("checkbox missing its value", function(){
        _imgui.checkbox("label");
    });
    ::_tThrows("sliderFloat missing its range", function(){
        _imgui.sliderFloat("label", 0.5);
    });
    ::_tThrows("labelText missing its value", function(){
        _imgui.labelText("label");
    });
    ::_tThrows("textColored missing components", function(){
        _imgui.textColored(1.0, "text");
    });
    ::_tThrows("dummy missing its height", function(){
        _imgui.dummy(10);
    });
});

_t("tooManyArguments", "Passing more arguments than a function takes is an error", function(){
    ::_tThrows("text with a spare argument", function(){
        _imgui.text("a", "b");
    });
    ::_tThrows("bullet with an argument", function(){
        _imgui.bullet("unexpected");
    });
    ::_tThrows("smallButton with a spare argument", function(){
        _imgui.smallButton("label", 10);
    });
});

_t("errorsLeaveTheApiUsable", "The api still works after a rejected call", function(){
    ::_tThrows("bad call", function(){
        _imgui.text(12345);
    });

    //The frame must not have been left in a broken state by the rejected call.
    _imgui.begin("errors/recovered");
    _imgui.text("still working");
    local pressed = _imgui.button("still working");
    _test.assertEqual("bool", typeof pressed);
    _imgui.end();

    _test.assertEqual("string", typeof _imgui.getVersion());
});

_t("badEnumValuesAreTolerated", "Out of range enum values do not crash", function(){
    _imgui.begin("errors/enums");
    //Flags are plain integers, so nothing stops a script passing nonsense.
    //imgui ignores bits it does not know, and the binding must not fall over.
    _imgui.begin("errors/enumWindow", 0x7FFFFF);
    _imgui.end();
    _imgui.treeNodeEx("enumNode", 0);
    _imgui.isItemHovered(0);
    _imgui.end();
});

_t("extremeNumbers", "Very large and very small numbers are handled", function(){
    _imgui.begin("errors/extremes");

    //Sizes and positions well outside the display.
    _imgui.setNextWindowPos(-10000, -10000);
    _imgui.begin("errors/offscreen");
    _imgui.end();

    //A zero-width range, where a slider has nothing to interpolate.
    _test.assertEqual(1.0, _imgui.sliderFloat("zeroRange", 1.0, 1.0, 1.0));
    //An inverted range.
    _imgui.sliderFloat("inverted", 0.5, 1.0, 0.0);
    //Large values.
    _imgui.dragFloat("large", 1.0e9);
    _imgui.dragInt("largeInt", 2000000000);

    _imgui.end();
});

_t("longStrings", "Long labels and text are handled", function(){
    local long = "";
    for(local i = 0; i < 100; i++) long += "0123456789";
    _test.assertEqual(1000, long.len());

    _imgui.begin("errors/longStrings");
    _imgui.text(long);
    _imgui.button(long);
    _imgui.setTooltip(long);
    _imgui.end();
});
