#pragma once

#include <nlohmann/json.hpp>
#include <string>

// ---------------------------------------------------------------------------
// EndpointInfo
//
// Nested sub-object to demonstrate JSON object nesting within a settings item.
// ---------------------------------------------------------------------------
struct EndpointInfo
{
    std::string host = "localhost";
    int         port = 8080;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EndpointInfo, host, port)
};

// ---------------------------------------------------------------------------
// NetworkConfig
//
// Demonstrates: nested object (EndpointInfo), int, bool fields.
// No DirtyTracker — plain POD-like aggregate.
// ---------------------------------------------------------------------------
struct NetworkConfig
{
    EndpointInfo endpoint  = {};
    int          timeoutMs = 5000;
    bool         enableTLS = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(NetworkConfig, endpoint, timeoutMs, enableTLS)
};
