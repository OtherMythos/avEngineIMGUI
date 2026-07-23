_t("pluginLoaded", "The plugin defines the _imgui namespace", function(){
    _test.assertTrue("_imgui" in getroottable());
    _test.assertEqual("string", typeof _imgui.getVersion());
});
