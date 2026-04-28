#pragma once

#include "settings/dirty_tracker.hpp"

#include <nlohmann/json.hpp>
#include <array>
#include <iostream>
#include <string>

// ---------------------------------------------------------------------------
// RenderSettings
//
// Inherits DirtyTracker so SettingsRegistry calls dirty() after every load.
// Overrides onChanged() to demonstrate custom side-effect notification.
// Demonstrates: array field, bool, string enum, int, DirtyTracker pattern.
// ---------------------------------------------------------------------------
struct RenderSettings : DirtyTracker
{
    std::array<float, 4> clearColor    = {0.1f, 0.1f, 0.1f, 1.0f};  // RGBA [0..1]
    bool                 wireframe     = false;
    std::string          shadowQuality = "medium";  // "low" | "medium" | "high"
    int                  maxLights     = 8;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RenderSettings,
        clearColor, wireframe, shadowQuality, maxLights)

protected:
    void onChanged() override
    {
        // Invoked automatically by DirtyTracker::dirty() on every load/set.
        std::cout << "[RenderSettings] Settings updated (modifiedCount="
                  << getModifiedCount() << ")\n";
    }
};
