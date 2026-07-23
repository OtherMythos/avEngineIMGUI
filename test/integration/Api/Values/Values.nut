//The value widgets. Squirrel has no reference primitives, so these take the
//current value and return the new one. Nothing is interacted with during the
//test, so every value must come back exactly as it went in, and every array
//must be left untouched.

_t("sliderFloat", "Value round-trips, with defaults and with every argument", function(){
    _imgui.begin("values/sliderFloat");

    local v = _imgui.sliderFloat("sliderFloat", 0.25, 0.0, 1.0);
    _test.assertEqual("float", typeof v);
    _test.assertEqual(0.25, v);

    //Explicit format and flags.
    _test.assertEqual(0.25, _imgui.sliderFloat("sliderFmt", 0.25, 0.0, 1.0, "%.1f"));
    _test.assertEqual(0.25, _imgui.sliderFloat("sliderFlags", 0.25, 0.0, 1.0, "%.3f", _imgui.SliderFlags_Logarithmic));
    //The format is optional and documented as accepting null.
    _test.assertEqual(0.25, _imgui.sliderFloat("sliderNullFmt", 0.25, 0.0, 1.0, null));
    //Integers where floats are expected.
    _test.assertEqual(1.0, _imgui.sliderFloat("sliderInts", 1, 0, 2));
    //Negative ranges.
    _test.assertEqual(-5.0, _imgui.sliderFloat("sliderNegative", -5.0, -10.0, 0.0));

    _imgui.end();
});

_t("sliderInt", "Integer slider round-trips", function(){
    _imgui.begin("values/sliderInt");

    local v = _imgui.sliderInt("sliderInt", 5, 0, 10);
    _test.assertEqual("integer", typeof v);
    _test.assertEqual(5, v);

    _test.assertEqual(5, _imgui.sliderInt("sliderIntFmt", 5, 0, 10, "%d units"));
    _test.assertEqual(5, _imgui.sliderInt("sliderIntFlags", 5, 0, 10, "%d", _imgui.SliderFlags_NoInput));
    _test.assertEqual(5, _imgui.sliderInt("sliderIntNullFmt", 5, 0, 10, null));
    _test.assertEqual(-3, _imgui.sliderInt("sliderIntNegative", -3, -10, 10));

    _imgui.end();
});

_t("sliderFloatN", "Array sliders accept 1 to 4 elements and report whether they changed", function(){
    _imgui.begin("values/sliderFloatN");

    foreach(count in [1, 2, 3, 4]){
        local values = [];
        for(local i = 0; i < count; i++) values.append(0.5);

        local changed = _imgui.sliderFloatN("sliderN" + count, values, 0.0, 1.0);
        _test.assertEqual("bool", typeof changed);
        _test.assertFalse(changed);
        //Untouched, so the array must be exactly as it was.
        _test.assertEqual(count, values.len());
        foreach(v in values) _test.assertEqual(0.5, v);
    }

    local formatted = [0.5, 0.5];
    _test.assertFalse(_imgui.sliderFloatN("sliderNFmt", formatted, 0.0, 1.0, "%.1f"));
    _test.assertFalse(_imgui.sliderFloatN("sliderNFlags", formatted, 0.0, 1.0, "%.3f", _imgui.SliderFlags_AlwaysClamp));
    _test.assertFalse(_imgui.sliderFloatN("sliderNNullFmt", formatted, 0.0, 1.0, null));

    _imgui.end();
});

_t("arraySizesAreValidated", "Wrongly sized arrays are errors rather than silent truncation", function(){
    _imgui.begin("values/badArrays");

    //Outside imgui's 1..4 component forms.
    ::_tThrows("empty array", function(){
        _imgui.sliderFloatN("sliderNEmpty", [], 0.0, 1.0);
    });
    ::_tThrows("five elements", function(){
        _imgui.sliderFloatN("sliderNFive", [0.1, 0.2, 0.3, 0.4, 0.5], 0.0, 1.0);
    });

    //Colour arrays must carry a component per channel, otherwise the binding
    //would read components that were never supplied.
    ::_tThrows("two component colour", function(){
        _imgui.colorEdit3("colorEdit3Short", [1.0, 0.5]);
    });
    ::_tThrows("three component rgba", function(){
        _imgui.colorEdit4("colorEdit4Short", [1.0, 0.5, 0.25]);
    });

    _imgui.end();
});

_t("dragFloat", "Drag float round-trips through all its optional arguments", function(){
    _imgui.begin("values/dragFloat");

    local v = _imgui.dragFloat("dragFloat", 1.5);
    _test.assertEqual("float", typeof v);
    _test.assertEqual(1.5, v);

    _test.assertEqual(1.5, _imgui.dragFloat("dragSpeed", 1.5, 0.1));
    _test.assertEqual(1.5, _imgui.dragFloat("dragRange", 1.5, 0.1, 0.0, 10.0));
    _test.assertEqual(1.5, _imgui.dragFloat("dragFmt", 1.5, 0.1, 0.0, 10.0, "%.2f"));
    _test.assertEqual(1.5, _imgui.dragFloat("dragFlags", 1.5, 0.1, 0.0, 10.0, "%.2f", _imgui.SliderFlags_Logarithmic));
    _test.assertEqual(1.5, _imgui.dragFloat("dragNullFmt", 1.5, 0.1, 0.0, 10.0, null));

    _imgui.end();
});

_t("dragInt", "Drag int round-trips through all its optional arguments", function(){
    _imgui.begin("values/dragInt");

    local v = _imgui.dragInt("dragInt", 7);
    _test.assertEqual("integer", typeof v);
    _test.assertEqual(7, v);

    _test.assertEqual(7, _imgui.dragInt("dragIntSpeed", 7, 0.5));
    _test.assertEqual(7, _imgui.dragInt("dragIntRange", 7, 0.5, 0, 100));
    _test.assertEqual(7, _imgui.dragInt("dragIntFmt", 7, 0.5, 0, 100, "%d%%"));
    _test.assertEqual(7, _imgui.dragInt("dragIntFlags", 7, 0.5, 0, 100, "%d", _imgui.SliderFlags_NoInput));
    _test.assertEqual(7, _imgui.dragInt("dragIntNullFmt", 7, 0.5, 0, 100, null));

    _imgui.end();
});

_t("inputFloat", "Input float round-trips", function(){
    _imgui.begin("values/inputFloat");

    local v = _imgui.inputFloat("inputFloat", 2.5);
    _test.assertEqual("float", typeof v);
    _test.assertEqual(2.5, v);

    _test.assertEqual(2.5, _imgui.inputFloat("inputFloatStep", 2.5, 0.1));
    _test.assertEqual(2.5, _imgui.inputFloat("inputFloatFast", 2.5, 0.1, 1.0));
    _test.assertEqual(2.5, _imgui.inputFloat("inputFloatFmt", 2.5, 0.1, 1.0, "%.4f"));
    _test.assertEqual(2.5, _imgui.inputFloat("inputFloatFlags", 2.5, 0.1, 1.0, "%.3f", _imgui.InputTextFlags_ReadOnly));
    _test.assertEqual(2.5, _imgui.inputFloat("inputFloatNullFmt", 2.5, 0.1, 1.0, null));

    _imgui.end();
});

_t("inputInt", "Input int round-trips", function(){
    _imgui.begin("values/inputInt");

    local v = _imgui.inputInt("inputInt", 42);
    _test.assertEqual("integer", typeof v);
    _test.assertEqual(42, v);

    _test.assertEqual(42, _imgui.inputInt("inputIntStep", 42, 1));
    _test.assertEqual(42, _imgui.inputInt("inputIntFast", 42, 1, 10));
    _test.assertEqual(42, _imgui.inputInt("inputIntFlags", 42, 1, 10, _imgui.InputTextFlags_ReadOnly));

    _imgui.end();
});

_t("inputText", "Text round-trips, including empty and long values", function(){
    _imgui.begin("values/inputText");

    local v = _imgui.inputText("inputText", "hello");
    _test.assertEqual("string", typeof v);
    _test.assertEqual("hello", v);

    _test.assertEqual("", _imgui.inputText("inputTextEmpty", ""));
    _test.assertEqual("flags", _imgui.inputText("inputTextFlags", "flags", _imgui.InputTextFlags_ReadOnly));

    //Something comfortably longer than a small stack buffer, to prove the
    //binding isn't truncating at some arbitrary size.
    local long = "";
    for(local i = 0; i < 200; i++) long += "abcdefghij";
    _test.assertEqual(2000, long.len());
    _test.assertEqual(long, _imgui.inputText("inputTextLong", long));

    _imgui.end();
});

_t("inputTextMultiline", "Multiline text round-trips, including newlines", function(){
    _imgui.begin("values/inputTextMultiline");

    local v = _imgui.inputTextMultiline("multiline", "line one\nline two");
    _test.assertEqual("string", typeof v);
    _test.assertEqual("line one\nline two", v);

    _test.assertEqual("sized", _imgui.inputTextMultiline("multilineSized", "sized", 200, 60));
    _test.assertEqual("flagged", _imgui.inputTextMultiline("multilineFlags", "flagged", 200, 60, _imgui.InputTextFlags_ReadOnly));

    _imgui.end();
});

_t("colorEdit3", "Colour components are left alone when untouched", function(){
    _imgui.begin("values/colorEdit3");

    local col = [1.0, 0.5, 0.25];
    local changed = _imgui.colorEdit3("colorEdit3", col);
    _test.assertEqual("bool", typeof changed);
    _test.assertFalse(changed);
    _test.assertEqual(3, col.len());
    _test.assertEqual(1.0, col[0]);
    _test.assertEqual(0.5, col[1]);
    _test.assertEqual(0.25, col[2]);

    _test.assertFalse(_imgui.colorEdit3("colorEdit3Flags", col, _imgui.ColorEditFlags_NoInputs));

    _imgui.end();
});

_t("colorEdit4", "Colour with alpha is left alone when untouched", function(){
    _imgui.begin("values/colorEdit4");

    local col = [1.0, 0.5, 0.25, 0.75];
    _test.assertFalse(_imgui.colorEdit4("colorEdit4", col));
    _test.assertEqual(4, col.len());
    _test.assertEqual(0.75, col[3]);

    _test.assertFalse(_imgui.colorEdit4("colorEdit4Flags", col, _imgui.ColorEditFlags_AlphaBar));

    _imgui.end();
});

_t("combo", "The selected index round-trips", function(){
    _imgui.begin("values/combo");

    local items = ["first", "second", "third"];
    local idx = _imgui.combo("combo", 1, items);
    _test.assertEqual("integer", typeof idx);
    _test.assertEqual(1, idx);

    //First and last index.
    _test.assertEqual(0, _imgui.combo("comboFirst", 0, items));
    _test.assertEqual(2, _imgui.combo("comboLast", 2, items));
    //Explicit popup height.
    _test.assertEqual(1, _imgui.combo("comboHeight", 1, items, 4));
    //A single item list.
    _test.assertEqual(0, _imgui.combo("comboSingle", 0, ["only"]));

    _imgui.end();
});

_t("listBox", "The selected index round-trips", function(){
    _imgui.begin("values/listBox");

    local items = ["alpha", "beta", "gamma"];
    local idx = _imgui.listBox("listBox", 2, items);
    _test.assertEqual("integer", typeof idx);
    _test.assertEqual(2, idx);

    _test.assertEqual(0, _imgui.listBox("listBoxHeight", 0, items, 2));
    _test.assertEqual(0, _imgui.listBox("listBoxSingle", 0, ["only"]));

    _imgui.end();
});

_t("emptyItemLists", "An empty item list is handled without reading past the end", function(){
    _imgui.begin("values/emptyItems");
    //-1 is imgui's "nothing selected"; it must survive an empty list.
    _test.assertEqual(-1, _imgui.combo("comboEmpty", -1, []));
    _test.assertEqual(-1, _imgui.listBox("listBoxEmpty", -1, []));
    _imgui.end();
});
