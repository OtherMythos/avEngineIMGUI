//An example of the draw list api: primitives drawn straight onto a window
//rather than assembled from widgets.
//Run with the avSetup.cfg in this directory (with plugin binaries in plugins/).
//
//The draw functions take screen space coordinates, the same space as
//getCursorScreenPos and getMousePos, so a script that wants to draw inside a
//window takes the cursor position as its origin and the content region as its
//size. An invisible button the size of the canvas makes it an item, so the
//hover and drag queries work over it like any widget.
//
//What is drawn: a timeline of coloured blocks on a grid, with a playhead that
//follows the mouse while the canvas is hovered. The same shape as a piano roll
//or a track view - the reason the api exists.

local state = {
    blocks = [
        //[row, start, length, r, g, b]
        [0, 0.0, 2.0, 0.35, 0.67, 0.92],
        [1, 1.0, 1.5, 0.92, 0.78, 0.35],
        [2, 2.5, 3.0, 0.55, 0.86, 0.55],
        [0, 3.0, 1.0, 0.35, 0.67, 0.92],
        [3, 0.5, 4.0, 0.86, 0.45, 0.55],
    ],
    rows = 4,
    length = 6.0,
    //Where the playhead sits, in timeline units; the mouse moves it.
    playhead = 1.5,
};

function drawTimeline(){
    _imgui.setNextWindowPos(40, 40, _imgui.Cond_FirstUseEver);
    _imgui.setNextWindowSize(640, 320, _imgui.Cond_FirstUseEver);
    if(_imgui.begin("Timeline")){
        _imgui.text("Hover the canvas to move the playhead. Everything below is the draw list.");

        local origin = _imgui.getCursorScreenPos();
        local size = _imgui.getContentRegionAvail();
        local x0 = origin[0]; local y0 = origin[1];
        local w = size[0]; local h = size[1];
        local x1 = x0 + w; local y1 = y0 + h;

        //The canvas is an item, so isItemHovered and the drag queries apply to it.
        _imgui.invisibleButton("##canvas", w, h);
        local hovered = _imgui.isItemHovered();

        //Nothing drawn below may spill outside the canvas.
        _imgui.pushClipRect(x0, y0, x1, y1);

        _imgui.drawRectFilled(x0, y0, x1, y1, 0.08, 0.08, 0.10, 1.0);

        local rowHeight = h / state.rows;
        local unit = w / state.length;
        //Row shading and the grid.
        for(local row = 0; row < state.rows; row++){
            local top = y0 + row * rowHeight;
            if(row % 2 == 1){
                _imgui.drawRectFilled(x0, top, x1, top + rowHeight, 0.12, 0.12, 0.15, 1.0);
            }
            _imgui.drawLine(x0, top, x1, top, 0.25, 0.25, 0.3, 1.0);
        }
        for(local i = 0; i <= state.length; i++){
            local x = x0 + i * unit;
            //Every second line is a bar line, drawn brighter and thicker.
            local bar = (i % 2 == 0);
            _imgui.drawLine(x, y0, x, y1, bar ? 0.5 : 0.3, bar ? 0.5 : 0.3, bar ? 0.55 : 0.35, 1.0, bar ? 2.0 : 1.0);
            _imgui.drawText(x + 3, y0 + 2, 0.7, 0.7, 0.75, 1.0, i.tostring());
        }

        //The blocks, with a rounded outline.
        foreach(b in state.blocks){
            local bx0 = x0 + b[1] * unit;
            local bx1 = x0 + (b[1] + b[2]) * unit;
            local by0 = y0 + b[0] * rowHeight + 4;
            local by1 = by0 + rowHeight - 8;
            _imgui.drawRectFilled(bx0, by0, bx1, by1, b[3], b[4], b[5], 0.85, 3.0);
            _imgui.drawRect(bx0, by0, bx1, by1, 1.0, 1.0, 1.0, 0.4, 3.0, 1.0);
        }

        //The playhead follows the mouse while the canvas is hovered.
        if(hovered){
            local mouse = _imgui.getMousePos();
            state.playhead = (mouse[0] - x0) / unit;
        }
        local px = x0 + state.playhead * unit;
        _imgui.drawLine(px, y0, px, y1, 1.0, 0.3, 0.3, 1.0, 2.0);
        _imgui.drawTriangleFilled(px - 6, y0, px + 6, y0, px, y0 + 8, 1.0, 0.3, 0.3, 1.0);
        _imgui.drawCircleFilled(px, y1 - 8, 5.0, 1.0, 0.3, 0.3, 1.0);

        _imgui.popClipRect();

        if(hovered){
            _imgui.setTooltip(format("playhead %.2f", state.playhead));
        }
    }
    _imgui.end();
}

function start(){
    print("imgui version: " + _imgui.getVersion());
}

function update(){
    //The engine can run several fixed updates per rendered frame; only build
    //the gui once per rendered frame.
    if(!_imgui.isFirstUpdateOfFrame()) return;
    drawTimeline();
}
