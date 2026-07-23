//The text drawing calls. None of them return anything, so these check that
//every form is accepted, including content that would break a printf-style api.

_t("text", "Plain text is drawn", function(){
    _imgui.begin("text/all");
    _imgui.text("plain text");
    _imgui.text("");
    _imgui.end();
});

_t("textFormatSpecifiers", "Text is never treated as a format string", function(){
    //If any of these were passed as a format string the engine would read
    //garbage arguments and likely crash.
    _imgui.begin("text/format");
    _imgui.text("100% sure");
    _imgui.text("%s %d %f %p %n");
    _imgui.textDisabled("%s%s%s");
    _imgui.textWrapped("%d%d%d");
    _imgui.bulletText("%x");
    _imgui.labelText("label%s", "value%d");
    _imgui.separatorText("%f");
    _imgui.end();
});

_t("textColored", "Coloured text takes four components and a string", function(){
    _imgui.begin("text/coloured");
    _imgui.textColored(1.0, 0.5, 0.25, 1.0, "coloured");
    //Integers are accepted where a number is expected.
    _imgui.textColored(1, 0, 0, 1, "integer components");
    _imgui.end();
});

_t("textVariants", "Disabled, wrapped and bullet text are drawn", function(){
    _imgui.begin("text/variants");
    _imgui.textDisabled("disabled");
    _imgui.textWrapped("a wrapped line that is long enough to need more than one row inside a narrow window");
    _imgui.bulletText("bulleted");
    _imgui.end();
});

_t("labelText", "Label text takes a label and a value", function(){
    _imgui.begin("text/label");
    _imgui.labelText("label", "value");
    _imgui.end();
});

_t("separatorText", "Separator text is drawn", function(){
    _imgui.begin("text/separator");
    _imgui.separatorText("section");
    _imgui.end();
});

_t("unicodeText", "Multi byte characters are accepted", function(){
    _imgui.begin("text/unicode");
    _imgui.text("héllo — ünicode ✓");
    _imgui.end();
});
