//Line and histogram plots.

_t("plotLines", "Plots accept an array with defaults and with every argument", function(){
    _imgui.begin("plots/lines");
    local values = [0.0, 0.5, 1.0, 0.75, 0.25, 0.6];

    _imgui.plotLines("##lines", values);
    _imgui.plotLines("##linesOverlay", values, "overlay text");
    //The overlay is optional and documented as accepting null.
    _imgui.plotLines("##linesNullOverlay", values, null);
    _imgui.plotLines("##linesScaled", values, null, 0.0, 1.0);
    _imgui.plotLines("##linesSized", values, "full", 0.0, 1.0, 200, 60);

    _imgui.end();
});

_t("plotHistogram", "Histograms take the same arguments as line plots", function(){
    _imgui.begin("plots/histogram");
    local values = [3.0, 1.0, 4.0, 1.0, 5.0, 9.0, 2.0];

    _imgui.plotHistogram("##hist", values);
    _imgui.plotHistogram("##histOverlay", values, "overlay");
    _imgui.plotHistogram("##histNullOverlay", values, null);
    _imgui.plotHistogram("##histScaled", values, null, 0.0, 10.0);
    _imgui.plotHistogram("##histSized", values, "full", 0.0, 10.0, 200, 60);

    _imgui.end();
});

_t("plotEdgeCases", "Empty, single value and negative data are handled", function(){
    _imgui.begin("plots/edges");

    //An empty array must not read past the start of the buffer.
    _imgui.plotLines("##empty", []);
    _imgui.plotHistogram("##emptyHist", []);

    _imgui.plotLines("##single", [1.0]);
    _imgui.plotHistogram("##singleHist", [1.0]);

    //Negative values, and a range that spans zero.
    _imgui.plotLines("##negative", [-1.0, 0.0, 1.0], null, -1.0, 1.0);
    _imgui.plotHistogram("##negativeHist", [-1.0, 0.0, 1.0], null, -1.0, 1.0);

    //A flat line, where imgui's auto scaling has no range to work with.
    _imgui.plotLines("##flat", [1.0, 1.0, 1.0]);

    _imgui.end();
});

_t("plotIntegerValues", "Integer arrays are accepted where floats are expected", function(){
    _imgui.begin("plots/integers");
    _imgui.plotLines("##ints", [1, 2, 3, 2, 1]);
    _imgui.plotHistogram("##intsHist", [1, 2, 3, 2, 1]);
    _imgui.end();
});

_t("plotLargeArray", "A large data set is plotted", function(){
    _imgui.begin("plots/large");
    local values = [];
    for(local i = 0; i < 1000; i++){
        values.append((i % 100).tofloat());
    }
    _imgui.plotLines("##large", values, null, 0.0, 100.0, 300, 80);
    _imgui.end();
});
