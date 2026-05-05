// ---------------------------------------------------------------------------
// settings_demo_main.cpp
//
// Demonstrates every feature of SettingsRegistry + SettingsItem:
//   1. Default values
//   2. loadJson    — deserializes from file; triggers DirtyTracker::dirty()
//   3. getItemValue — typed read of a single member
//   4. setItemValue — typed write of a single member; triggers dirty()
//   5. getItems    — iterate the registry
//   6. saveJson    — serialize all items back to file
// ---------------------------------------------------------------------------
#include "settings/examples/app_config.hpp"
#include "settings/examples/network_config.hpp"
#include "settings/examples/render_settings.hpp"
#include "settings/settings_item.hpp"

#include <cassert>
#include <iostream>

// File-scope registrations — each SettingsItem<T> registers itself at
// static-init time under the given string key.
SettingsItem<AppConfig>      g_app("AppConfig");
SettingsItem<RenderSettings> g_render("RenderSettings"); // inherits DirtyTracker
SettingsItem<NetworkConfig>  g_network("NetworkConfig");

int main()
{
    auto& reg = SettingsRegistry::instance();

    // ------------------------------------------------------------------
    // Step 1: Default values
    // ------------------------------------------------------------------
    std::cout << "=== Step 1: Default values ===\n";
    assert(g_app->appName == "MyApp");
    assert(g_app->windowWidth == 1280);
    assert(g_app->windowHeight == 720);
    assert(g_app->fullscreen == false);
    assert(g_app->targetFps == 60.0f);
    std::cout << "  appName:      " << g_app->appName << "\n";
    std::cout << "  windowWidth:  " << g_app->windowWidth << "\n";
    std::cout << "  fullscreen:   " << g_app->fullscreen << "\n";

    assert(g_render->shadowQuality == "medium");
    assert(g_render->maxLights == 8);
    assert(g_render->getModifiedCount() == 0U);
    std::cout << "  shadowQuality: " << g_render->shadowQuality << "\n";
    std::cout << "  maxLights:     " << g_render->maxLights << "\n";
    std::cout << "  modifiedCount: " << g_render->getModifiedCount() << "\n";

    assert(g_network->endpoint.host == "localhost");
    assert(g_network->endpoint.port == 8080);
    std::cout << "  endpoint:     " << g_network->endpoint.host << ":" << g_network->endpoint.port << "\n";

    // ------------------------------------------------------------------
    // Step 2: loadJson — reads settings.json, fires dirty() on RenderSettings
    // ------------------------------------------------------------------
    std::cout << "\n=== Step 2: loadJson ===\n";
    reg.loadJson("settings.json");
    // RenderSettings inherits DirtyTracker; modifiedCount increments on load.
    assert(g_render->getModifiedCount() == 1U);
    std::cout << "  appName:       " << g_app->appName << "\n";
    std::cout << "  shadowQuality: " << g_render->shadowQuality << "\n";
    std::cout << "  modifiedCount: " << g_render->getModifiedCount() << "\n";
    std::cout << "  endpoint:      " << g_network->endpoint.host << ":" << g_network->endpoint.port << "\n";

    // ------------------------------------------------------------------
    // Step 3: getItemValue — typed read of a single member
    // ------------------------------------------------------------------
    std::cout << "\n=== Step 3: getItemValue ===\n";
    int  width    = 0;
    bool gotWidth = reg.getItemValue<int>("AppConfig", "windowWidth", width);
    assert(gotWidth);
    std::cout << "  AppConfig.windowWidth = " << width << "\n";

    std::string shadow;
    bool        gotShadow = reg.getItemValue<std::string>("RenderSettings", "shadowQuality", shadow);
    assert(gotShadow);
    std::cout << "  RenderSettings.shadowQuality = " << shadow << "\n";

    // ------------------------------------------------------------------
    // Step 4: setItemValue — typed write; DirtyTracker fires again
    // ------------------------------------------------------------------
    std::cout << "\n=== Step 4: setItemValue ===\n";

    bool ok = reg.setItemValue<std::string>("AppConfig", "appName", "DemoApp");
    assert(ok);
    assert(g_app->appName == "DemoApp");
    std::cout << "  AppConfig.appName -> " << g_app->appName << "\n";

    ok = reg.setItemValue<std::string>("RenderSettings", "shadowQuality", "high");
    assert(ok);
    assert(g_render->shadowQuality == "high");
    assert(g_render->getModifiedCount() == 2U);
    std::cout << "  RenderSettings.shadowQuality -> " << g_render->shadowQuality << "\n";
    std::cout << "  RenderSettings.modifiedCount  = " << g_render->getModifiedCount() << "\n";

    ok = reg.setItemValue<bool>("AppConfig", "fullscreen", true);
    assert(ok);
    assert(g_app->fullscreen == true);
    std::cout << "  AppConfig.fullscreen -> " << g_app->fullscreen << "\n";

    // ------------------------------------------------------------------
    // Step 5: getItems — iterate the full registry
    // ------------------------------------------------------------------
    std::cout << "\n=== Step 5: getItems (iterate registry) ===\n";
    for (const auto& [key, entry] : reg.getItems())
    {
        std::cout << "  [" << key << "]\n";
        for (auto& [member, _] : entry.saver(entry.ptr).items())
            std::cout << "    " << member << "\n";
    }

    // ------------------------------------------------------------------
    // Step 6: saveJson — serialize current state to file
    // ------------------------------------------------------------------
    std::cout << "\n=== Step 6: saveJson ===\n";
    reg.saveJson("out.json");
    std::cout << "  Saved current state to out.json\n";

    std::cout << "\nAll assertions passed.\n";
    return 0;
}
