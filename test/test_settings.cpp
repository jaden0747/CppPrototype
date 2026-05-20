// ---------------------------------------------------------------------------
// test_settings.cpp — Unit tests for SettingsRegistry + SettingsItem
// ---------------------------------------------------------------------------
#include "settings/settings_item.hpp"
#include "settings/examples/app_config.hpp"
#include "settings/examples/render_settings.hpp"
#include "settings/examples/network_config.hpp"

#include <gtest/gtest.h>
#include <fstream>

// Note: SettingsRegistry is a singleton, so tests share state.
// We use a single global registration.
static SettingsItem<AppConfig>      g_app("AppConfig");
static SettingsItem<RenderSettings> g_render("RenderSettings");
static SettingsItem<NetworkConfig>  g_network("NetworkConfig");

TEST(Settings, DefaultValues)
{
    EXPECT_EQ(g_app->appName, "MyApp");
    EXPECT_EQ(g_app->windowWidth, 1280);
    EXPECT_EQ(g_app->windowHeight, 720);
    EXPECT_FALSE(g_app->fullscreen);
    EXPECT_FLOAT_EQ(g_app->targetFps, 60.0f);
}

TEST(Settings, LoadJson)
{
    // Write a temporary JSON file
    const char* json = R"({
        "AppConfig": {
            "appName": "TestApp",
            "windowWidth": 800,
            "windowHeight": 600,
            "fullscreen": true,
            "targetFps": 30.0
        },
        "RenderSettings": {
            "clearColor": [1.0, 0.0, 0.0, 1.0],
            "wireframe": true,
            "shadowQuality": "low",
            "maxLights": 4
        }
    })";

    {
        std::ofstream f("test_settings_tmp.json");
        f << json;
    }

    SettingsRegistry::instance().loadJson("test_settings_tmp.json");

    EXPECT_EQ(g_app->appName, "TestApp");
    EXPECT_EQ(g_app->windowWidth, 800);
    EXPECT_TRUE(g_app->fullscreen);
    EXPECT_FLOAT_EQ(g_app->targetFps, 30.0f);

    EXPECT_TRUE(g_render->wireframe);
    EXPECT_EQ(g_render->shadowQuality, "low");
    EXPECT_EQ(g_render->maxLights, 4);
    EXPECT_GT(g_render->getModifiedCount(), 0u);

    std::remove("test_settings_tmp.json");
}

TEST(Settings, SetItemValue)
{
    auto& reg = SettingsRegistry::instance();

    bool ok = reg.setItemValue<std::string>("AppConfig", "appName", "Modified");
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_app->appName, "Modified");

    ok = reg.setItemValue<int>("AppConfig", "windowWidth", 1920);
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_app->windowWidth, 1920);

    // Non-existent key
    ok = reg.setItemValue<int>("AppConfig", "nonExistent", 0);
    EXPECT_FALSE(ok);
}

TEST(Settings, GetItemValue)
{
    auto& reg = SettingsRegistry::instance();

    std::string name;
    bool ok = reg.getItemValue<std::string>("AppConfig", "appName", name);
    EXPECT_TRUE(ok);
    EXPECT_EQ(name, g_app->appName);
}

TEST(Settings, DirtyTracker)
{
    uint32_t before = g_render->getModifiedCount();
    auto& reg = SettingsRegistry::instance();
    reg.setItemValue<bool>("RenderSettings", "wireframe", !g_render->wireframe);
    EXPECT_EQ(g_render->getModifiedCount(), before + 1);
}

TEST(Settings, SaveAndReload)
{
    auto& reg = SettingsRegistry::instance();

    reg.setItemValue<std::string>("AppConfig", "appName", "SaveTest");
    reg.saveJson("test_save_tmp.json");

    // Modify in memory
    reg.setItemValue<std::string>("AppConfig", "appName", "Changed");
    EXPECT_EQ(g_app->appName, "Changed");

    // Reload
    reg.loadJson("test_save_tmp.json");
    EXPECT_EQ(g_app->appName, "SaveTest");

    std::remove("test_save_tmp.json");
}
