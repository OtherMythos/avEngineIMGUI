//Layout, cursor and measurement helpers.

_t("spacingCalls", "The parameterless layout calls are accepted", function(){
    _imgui.begin("layout/spacing");
    _imgui.text("a");
    _imgui.spacing();
    _imgui.separator();
    _imgui.newLine();
    _imgui.text("b");
    _imgui.end();
});

_t("sameLine", "sameLine keeps items on one row, with and without arguments", function(){
    _imgui.begin("layout/sameLine");

    _imgui.text("first");
    _imgui.sameLine();
    _imgui.text("second");

    //Explicit offset, and offset plus spacing.
    _imgui.text("third");
    _imgui.sameLine(120);
    _imgui.text("fourth");
    _imgui.text("fifth");
    _imgui.sameLine(0, 40);
    _imgui.text("sixth");

    _imgui.end();
});

_t("sameLineKeepsY", "Items after sameLine share a row", function(){
    _imgui.begin("layout/sameLineY");
    _imgui.text("left");
    local firstY = _imgui.getCursorPosY();
    _imgui.sameLine();
    local secondY = _imgui.getCursorPosY();
    _imgui.text("right");
    _imgui.end();

    //sameLine rewinds the cursor to the previous line.
    _test.assertTrue(secondY < firstY);
});

_t("dummy", "Dummy reserves space", function(){
    _imgui.begin("layout/dummy");
    local before = _imgui.getCursorPosY();
    _imgui.dummy(40, 60);
    local after = _imgui.getCursorPosY();
    _imgui.end();
    _test.assertTrue(after > before);
});

_t("indent", "Indent and unindent move the cursor and are reversible", function(){
    _imgui.begin("layout/indent");

    local startX = _imgui.getCursorPosX();
    _imgui.indent();
    local indented = _imgui.getCursorPosX();
    _imgui.unindent();
    local restored = _imgui.getCursorPosX();

    _test.assertTrue(indented > startX);
    ::_tNear(restored, startX, 0.01);

    //Explicit widths.
    _imgui.indent(50);
    local wide = _imgui.getCursorPosX();
    _imgui.unindent(50);
    _test.assertTrue(wide > startX);
    ::_tNear(_imgui.getCursorPosX(), startX, 0.01);

    _imgui.end();
});

_t("group", "Groups open and close", function(){
    _imgui.begin("layout/group");
    _imgui.beginGroup();
    _imgui.text("inside a group");
    _imgui.text("still inside");
    _imgui.endGroup();
    _imgui.end();
});

_t("itemWidth", "Item width can be set for one item or pushed for many", function(){
    _imgui.begin("layout/itemWidth");

    _imgui.setNextItemWidth(80);
    _imgui.sliderFloat("narrow", 0.5, 0.0, 1.0);

    _imgui.pushItemWidth(120);
    _imgui.sliderFloat("pushedA", 0.5, 0.0, 1.0);
    _imgui.sliderFloat("pushedB", 0.5, 0.0, 1.0);
    _imgui.popItemWidth();

    _imgui.end();
});

_t("cursorPosition", "Cursor position can be read and written", function(){
    _imgui.setNextWindowSize(300, 200, _imgui.Cond_Always);
    _imgui.begin("layout/cursor");

    _imgui.setCursorPos(40, 50);
    local x = _imgui.getCursorPosX();
    local y = _imgui.getCursorPosY();
    _test.assertEqual("float", typeof x);
    _test.assertEqual("float", typeof y);
    ::_tNear(x, 40.0, 0.01);
    ::_tNear(y, 50.0, 0.01);

    local screen = _imgui.getCursorScreenPos();
    _test.assertEqual("array", typeof screen);
    _test.assertEqual(2, screen.len());

    //imgui asserts if setCursorPos moves the cursor past the window edge and no
    //item is submitted to grow it, so submit one before ending the window.
    _imgui.text("item at the moved cursor");

    _imgui.end();
});

_t("calcTextSize", "Text measurement grows with the string and respects wrapping", function(){
    _imgui.begin("layout/calcText");

    local small = _imgui.calcTextSize("hi");
    local large = _imgui.calcTextSize("a much longer piece of text");
    _test.assertEqual("array", typeof small);
    _test.assertEqual(2, small.len());
    _test.assertTrue(small[0] > 0.0);
    _test.assertTrue(small[1] > 0.0);
    _test.assertTrue(large[0] > small[0]);
    //Same font, so one unwrapped line is the same height.
    ::_tNear(large[1], small[1], 0.01);

    //An empty string still has line height.
    local empty = _imgui.calcTextSize("");
    _test.assertTrue(empty[1] > 0.0);

    //With a wrap width the text is forced onto more rows.
    local wrapped = _imgui.calcTextSize("a much longer piece of text", 40.0);
    _test.assertTrue(wrapped[1] > large[1]);
    _test.assertTrue(wrapped[0] <= large[0]);

    _imgui.end();
});

_t("getFrameHeight", "Frame height is a positive float", function(){
    _imgui.begin("layout/frameHeight");
    local h = _imgui.getFrameHeight();
    _imgui.end();
    _test.assertEqual("float", typeof h);
    _test.assertTrue(h > 0.0);
});

_t("alignTextToFramePadding", "Text can be aligned to framed widgets", function(){
    _imgui.begin("layout/align");
    _imgui.alignTextToFramePadding();
    _imgui.text("label");
    _imgui.sameLine();
    _imgui.button("framed");
    _imgui.end();
});

_t("disabled", "Disabled blocks open and close, and nest", function(){
    _imgui.begin("layout/disabled");

    _imgui.beginDisabled();
    _test.assertFalse(_imgui.button("disabledButton"));
    _imgui.endDisabled();

    //The flag argument is optional; false leaves items enabled.
    _imgui.beginDisabled(false);
    _imgui.button("stillEnabled");
    _imgui.endDisabled();

    _imgui.beginDisabled(true);
    _imgui.beginDisabled(true);
    _imgui.text("nested");
    _imgui.endDisabled();
    _imgui.endDisabled();

    _imgui.end();
});
