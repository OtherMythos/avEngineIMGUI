//Tree nodes and collapsing headers. treePop must only follow an open node,
//so these drive the open state explicitly rather than relying on defaults.

_t("treeNodeClosed", "A closed node returns false and must not be popped", function(){
    _imgui.begin("trees/closed");
    _imgui.setNextItemOpen(false, _imgui.Cond_Always);
    local open = _imgui.treeNode("closedNode");
    _test.assertEqual("bool", typeof open);
    _test.assertFalse(open);
    _imgui.end();
});

_t("treeNodeOpen", "An open node returns true and is popped", function(){
    _imgui.begin("trees/open");
    _imgui.setNextItemOpen(true, _imgui.Cond_Always);
    local open = _imgui.treeNode("openNode");
    _test.assertTrue(open);
    if(open){
        _imgui.text("inside the node");
        _imgui.treePop();
    }
    _imgui.end();
});

_t("treeNodeEx", "treeNodeEx accepts flags, including ones that skip the push", function(){
    _imgui.begin("trees/ex");

    _imgui.setNextItemOpen(true, _imgui.Cond_Always);
    local open = _imgui.treeNodeEx("exNode");
    _test.assertEqual("bool", typeof open);
    if(open) _imgui.treePop();

    //DefaultOpen opens without setNextItemOpen.
    local defaultOpen = _imgui.treeNodeEx("exDefaultOpen", _imgui.TreeNodeFlags_DefaultOpen);
    _test.assertTrue(defaultOpen);
    if(defaultOpen) _imgui.treePop();

    //Leaf and NoTreePushOnOpen do not push, so they are never popped.
    _imgui.treeNodeEx("exLeaf", _imgui.TreeNodeFlags_Leaf | _imgui.TreeNodeFlags_NoTreePushOnOpen);
    _imgui.treeNodeEx("exBullet", _imgui.TreeNodeFlags_Bullet | _imgui.TreeNodeFlags_NoTreePushOnOpen);

    //Combined flags.
    local framed = _imgui.treeNodeEx("exFramed", _imgui.TreeNodeFlags_Framed | _imgui.TreeNodeFlags_DefaultOpen | _imgui.TreeNodeFlags_SpanAvailWidth);
    if(framed) _imgui.treePop();

    _imgui.end();
});

_t("collapsingHeader", "Headers report their open state and need no pop", function(){
    _imgui.begin("trees/header");

    _imgui.setNextItemOpen(true, _imgui.Cond_Always);
    local open = _imgui.collapsingHeader("openHeader");
    _test.assertEqual("bool", typeof open);
    _test.assertTrue(open);

    _imgui.setNextItemOpen(false, _imgui.Cond_Always);
    _test.assertFalse(_imgui.collapsingHeader("closedHeader"));

    //With flags.
    local flagged = _imgui.collapsingHeader("flaggedHeader", _imgui.TreeNodeFlags_DefaultOpen);
    _test.assertTrue(flagged);

    _imgui.end();
});

_t("setNextItemOpen", "The condition argument is optional", function(){
    _imgui.begin("trees/setOpen");
    //No condition supplied.
    _imgui.setNextItemOpen(true);
    local open = _imgui.treeNode("noCondNode");
    if(open) _imgui.treePop();

    foreach(cond in [_imgui.Cond_Always, _imgui.Cond_Once, _imgui.Cond_FirstUseEver, _imgui.Cond_Appearing]){
        _imgui.setNextItemOpen(false, cond);
        local o = _imgui.treeNode("condNode" + cond);
        if(o) _imgui.treePop();
    }
    _imgui.end();
});

_t("nestedTrees", "Nested nodes push and pop in order", function(){
    _imgui.begin("trees/nested");
    _imgui.setNextItemOpen(true, _imgui.Cond_Always);
    if(_imgui.treeNode("outer")){
        _imgui.setNextItemOpen(true, _imgui.Cond_Always);
        if(_imgui.treeNode("inner")){
            _imgui.text("deep");
            _imgui.treePop();
        }
        _imgui.treePop();
    }
    _imgui.end();
});
