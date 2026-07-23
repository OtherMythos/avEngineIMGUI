//Tables. endTable is only legal when beginTable returned true.

_t("beginTable", "A table opens with the minimum arguments", function(){
    _imgui.begin("tables/basic");
    local open = _imgui.beginTable("basicTable", 2);
    _test.assertEqual("bool", typeof open);
    _test.assertTrue(open);
    if(open){
        _imgui.tableNextRow();
        _imgui.tableNextColumn();
        _imgui.text("a");
        _imgui.tableNextColumn();
        _imgui.text("b");
        _imgui.endTable();
    }
    _imgui.end();
});

_t("tableFlagsAndSize", "Flags and an outer size are accepted", function(){
    _imgui.begin("tables/flags");
    local flags = _imgui.TableFlags_Borders
                | _imgui.TableFlags_RowBg
                | _imgui.TableFlags_Resizable
                | _imgui.TableFlags_SizingStretchProp;

    if(_imgui.beginTable("flagTable", 3, flags)){
        _imgui.tableNextRow();
        _imgui.tableNextColumn();
        _imgui.text("cell");
        _imgui.endTable();
    }

    //With an explicit outer size.
    if(_imgui.beginTable("sizedTable", 2, flags, 300, 120)){
        _imgui.tableNextRow();
        _imgui.tableNextColumn();
        _imgui.text("sized");
        _imgui.endTable();
    }
    _imgui.end();
});

_t("tableHeaders", "Columns can be set up and a header row emitted", function(){
    _imgui.begin("tables/headers");
    if(_imgui.beginTable("headerTable", 3, _imgui.TableFlags_Borders)){
        _imgui.tableSetupColumn("name");
        _imgui.tableSetupColumn("value", _imgui.TableColumnFlags_WidthStretch);
        _imgui.tableSetupColumn("fixed", _imgui.TableColumnFlags_WidthFixed, 80);
        _imgui.tableHeadersRow();

        _imgui.tableNextRow();
        _imgui.tableNextColumn();
        _imgui.text("row");
        _imgui.endTable();
    }
    _imgui.end();
});

_t("tableNextColumn", "tableNextColumn reports visibility and wraps rows", function(){
    _imgui.begin("tables/columns");
    if(_imgui.beginTable("columnTable", 2, _imgui.TableFlags_Borders)){
        _imgui.tableNextRow();

        local first = _imgui.tableNextColumn();
        _test.assertEqual("bool", typeof first);
        _test.assertTrue(first);
        _imgui.text("c0");

        _test.assertTrue(_imgui.tableNextColumn());
        _imgui.text("c1");

        //Past the last column it wraps onto a new row.
        _test.assertTrue(_imgui.tableNextColumn());
        _imgui.text("c0 of the next row");

        _imgui.endTable();
    }
    _imgui.end();
});

_t("tableSetColumnIndex", "Columns can be addressed directly", function(){
    _imgui.begin("tables/setIndex");
    if(_imgui.beginTable("indexTable", 3, _imgui.TableFlags_Borders)){
        _imgui.tableNextRow();

        local ok = _imgui.tableSetColumnIndex(2);
        _test.assertEqual("bool", typeof ok);
        _test.assertTrue(ok);
        _imgui.text("third column");

        _test.assertTrue(_imgui.tableSetColumnIndex(0));
        _imgui.text("back to the first");

        _imgui.endTable();
    }
    _imgui.end();
});

_t("tableNextRowArguments", "Row flags and a minimum height are accepted", function(){
    _imgui.begin("tables/rows");
    if(_imgui.beginTable("rowTable", 1, _imgui.TableFlags_Borders)){
        _imgui.tableNextRow(0);
        _imgui.tableNextColumn();
        _imgui.text("plain row");

        _imgui.tableNextRow(0, 40);
        _imgui.tableNextColumn();
        _imgui.text("tall row");

        _imgui.endTable();
    }
    _imgui.end();
});

_t("scrollingTable", "A scrolling table with many rows", function(){
    _imgui.begin("tables/scrolling");
    local flags = _imgui.TableFlags_ScrollY | _imgui.TableFlags_RowBg | _imgui.TableFlags_Borders;
    if(_imgui.beginTable("scrollTable", 2, flags, 200, 100)){
        _imgui.tableSetupColumn("index");
        _imgui.tableSetupColumn("value");
        _imgui.tableHeadersRow();
        for(local i = 0; i < 50; i++){
            _imgui.tableNextRow();
            _imgui.tableNextColumn();
            _imgui.text(i.tostring());
            _imgui.tableNextColumn();
            _imgui.text("row " + i);
        }
        _imgui.endTable();
    }
    _imgui.end();
});
