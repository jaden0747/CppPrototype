#pragma once

#include <nlohmann/json.hpp>
#include <string>

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
    bool        fullscreen   = false;
    float       targetFps    = 60.0f;
    float       uiFontSize   = 16.0f;
    std::string uiFontPath   = "resources/font/ComicMonoNF-Regular.ttf";

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppConfig,
        appName, windowWidth, windowHeight, fullscreen, targetFps, uiFontSize, uiFontPath)
};
