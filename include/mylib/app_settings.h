#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include "settings/settings_item.hpp"

// ---------------------------------------------------------------------------
// AppConfig
//
// Flat settings struct with scalar fields of different types.
// Does NOT inherit DirtyTracker, so no change-count bookkeeping.
// Demonstrates: bool, int, float, string fields.
// ---------------------------------------------------------------------------
struct AppConfig
{
    std::string appName      = "MyApp";
    int         windowWidth  = 1280;
    int         windowHeight = 720;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppConfig,
        appName, windowWidth, windowHeight)
};

extern SettingsItem<AppConfig> g_appConfig;
