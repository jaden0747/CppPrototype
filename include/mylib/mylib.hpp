#pragma once

#include <iostream>
#include <string>

#include "settings/dirty_tracker.hpp"
#include "settings/settings_item.hpp"

struct AppSettings : DirtyTracker
{
    int width  = 1280;
    int height = 720;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppSettings, width, height);

protected:
    void onChanged() override
    {
        std::cout << "[AppSettings] Changed: width=" << width << ", height=" << height << "\n";
    }
};

extern SettingsItem<AppSettings> g_AppSettings;

class Main
{
public:
    static void run();
};
