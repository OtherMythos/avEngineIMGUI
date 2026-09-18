//Primitives on the current window's draw list. Nothing can be read back from
//a draw list, so these check that every call is accepted with its defaults and
//with every argument, inside a window and clipped, and that the argument
//checks reject the wrong types rather than crashing.

_t("drawListPrimitives", "Every draw call accepts its required arguments", function(){
    _imgui.begin("drawList/primitives");
    local o = _imgui.getCursorScreenPos();
    local x = o[0]; local y = o[1];

    _imgui.drawLine(x, y, x + 100, y + 50, 1.0, 0.0, 0.0, 1.0);
    _imgui.drawRect(x, y, x + 100, y + 50, 0.0, 1.0, 0.0, 1.0);
    _imgui.drawRectFilled(x, y, x + 100, y + 50, 0.0, 0.0, 1.0, 1.0);
    _imgui.drawTriangleFilled(x, y, x + 100, y, x + 50, y + 50, 1.0, 1.0, 0.0, 1.0);
    _imgui.drawCircleFilled(x + 50, y + 25, 10.0, 0.0, 1.0, 1.0, 1.0);
    _imgui.drawText(x, y, 1.0, 1.0, 1.0, 1.0, "draw list text");

    _imgui.end();
});

_t("drawListOptionalArguments", "Thickness and rounding are optional", function(){
    _imgui.begin("drawList/optional");
    local o = _imgui.getCursorScreenPos();
    local x = o[0]; local y = o[1];

    _imgui.drawLine(x, y, x + 100, y + 50, 1.0, 0.0, 0.0, 1.0, 3.0);
    _imgui.drawRect(x, y, x + 100, y + 50, 0.0, 1.0, 0.0, 1.0, 4.0);
    _imgui.drawRect(x, y, x + 100, y + 50, 0.0, 1.0, 0.0, 1.0, 4.0, 2.0);
    _imgui.drawRectFilled(x, y, x + 100, y + 50, 0.0, 0.0, 1.0, 1.0, 4.0);

    _imgui.end();
});

_t("drawListIntegerArguments", "Integer coordinates and colours are accepted where numbers are expected", function(){
    _imgui.begin("drawList/integers");
    _imgui.drawLine(10, 10, 100, 50, 1, 0, 0, 1);
    _imgui.drawRectFilled(10, 10, 100, 50, 0, 0, 1, 1, 2);
    _imgui.drawCircleFilled(50, 25, 10, 0, 1, 1, 1);
    _imgui.drawText(10, 10, 1, 1, 1, 1, "ints");
    _imgui.end();
});

_t("drawListClipRect", "A clip rectangle can be pushed and popped, intersecting or not", function(){
    _imgui.begin("drawList/clip");
    local o = _imgui.getCursorScreenPos();
    local x = o[0]; local y = o[1];

    _imgui.pushClipRect(x, y, x + 50, y + 50);
    _imgui.drawRectFilled(x - 100, y - 100, x + 200, y + 200, 1.0, 0.0, 0.0, 1.0);
    _imgui.popClipRect();

    _imgui.pushClipRect(x, y, x + 50, y + 50, false);
    _imgui.drawLine(x - 100, y, x + 200, y, 0.0, 1.0, 0.0, 1.0);
    _imgui.popClipRect();

    //Nested.
    _imgui.pushClipRect(x, y, x + 100, y + 100);
    _imgui.pushClipRect(x + 10, y + 10, x + 50, y + 50, true);
    _imgui.drawCircleFilled(x + 30, y + 30, 40.0, 0.0, 0.0, 1.0, 1.0);
    _imgui.popClipRect();
    _imgui.popClipRect();

    _imgui.end();
});

_t("drawListOutsideWindow", "Drawing before any begin lands on imgui's fallback window rather than crashing", function(){
    _imgui.drawLine(0, 0, 10, 10, 1.0, 1.0, 1.0, 1.0);
    _imgui.drawText(0, 0, 1.0, 1.0, 1.0, 1.0, "fallback");
});

_t("drawListArgumentValidation", "Wrong types and missing arguments are squirrel errors", function(){
    _imgui.begin("drawList/errors");
    _tThrows("drawLine with too few arguments", function(){ _imgui.drawLine(0, 0, 10, 10); });
    _tThrows("drawLine with a string coordinate", function(){ _imgui.drawLine("0", 0, 10, 10, 1, 1, 1, 1); });
    _tThrows("drawRectFilled with a string colour", function(){ _imgui.drawRectFilled(0, 0, 10, 10, "red", 0, 0, 1); });
    _tThrows("drawText without text", function(){ _imgui.drawText(0, 0, 1, 1, 1, 1); });
    _tThrows("drawText with a number for text", function(){ _imgui.drawText(0, 0, 1, 1, 1, 1, 5); });
    _tThrows("drawCircleFilled with too few arguments", function(){ _imgui.drawCircleFilled(0, 0, 5); });
    _tThrows("drawTriangleFilled with too few arguments", function(){ _imgui.drawTriangleFilled(0, 0, 1, 1, 2, 2); });
    _tThrows("pushClipRect with a string flag", function(){ _imgui.pushClipRect(0, 0, 10, 10, "yes"); });
    _imgui.end();
});
