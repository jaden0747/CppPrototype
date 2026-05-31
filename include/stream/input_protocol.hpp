#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

namespace stream
{

// ---------------------------------------------------------------------------
// Input event protocol — sent from client (Moonlight) to server (Sunshine)
// over a UDP channel in the reverse direction from the video stream.
//
// Wire format: [2-byte big-endian JSON length][JSON payload]
// Max event size: 1024 bytes (well within a single UDP datagram).
//
// Design decision: UDP was chosen over TCP for input because:
//   - Input events are small and frequent (mouse moves at ~125Hz)
//   - A single dropped mouse-move is irrelevant; the next one supersedes it
//   - TCP head-of-line blocking would add latency to all events
//   - This mirrors Moonlight's approach (separate UDP channel for input)
// ---------------------------------------------------------------------------

// Event types matching the roadmap specification
enum class InputEventType : uint8_t
{
    KeyDown    = 1,
    KeyUp      = 2,
    MouseMove  = 3,
    MouseClick = 4,
};

inline const char* to_string(InputEventType t)
{
    switch (t)
    {
    case InputEventType::KeyDown: return "key_down";
    case InputEventType::KeyUp: return "key_up";
    case InputEventType::MouseMove: return "mouse_move";
    case InputEventType::MouseClick: return "mouse_click";
    }
    return "unknown";
}

inline InputEventType input_event_type_from_string(const std::string& s)
{
    if (s == "key_down") return InputEventType::KeyDown;
    if (s == "key_up") return InputEventType::KeyUp;
    if (s == "mouse_move") return InputEventType::MouseMove;
    if (s == "mouse_click") return InputEventType::MouseClick;
    return InputEventType::KeyDown; // fallback
}

// ---------------------------------------------------------------------------
// Input events
// ---------------------------------------------------------------------------

struct KeyEvent
{
    std::string type; // "key_down" or "key_up"
    std::string key;  // key name (e.g. "a", "Key.space", "Key.enter")

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(KeyEvent, type, key)
};

struct MouseMoveEvent
{
    std::string type = "mouse_move";
    int         x    = 0;
    int         y    = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MouseMoveEvent, type, x, y)
};

struct MouseClickEvent
{
    std::string type    = "mouse_click";
    int         x       = 0;
    int         y       = 0;
    std::string button  = "left";
    bool        pressed = false;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(MouseClickEvent, type, x, y, button, pressed)
};

// ---------------------------------------------------------------------------
// Generic input event (union-like JSON wrapper)
// ---------------------------------------------------------------------------

struct InputEvent
{
    std::string    type;
    nlohmann::json data;

    // Construct from specific event types
    static InputEvent from_key(const std::string& event_type, const std::string& key)
    {
        InputEvent ev;
        ev.type       = event_type;
        ev.data       = {{"type", event_type}, {"key", key}};
        return ev;
    }

    static InputEvent from_mouse_move(int x, int y)
    {
        InputEvent ev;
        ev.type = "mouse_move";
        ev.data = {{"type", "mouse_move"}, {"x", x}, {"y", y}};
        return ev;
    }

    static InputEvent from_mouse_click(int x, int y, const std::string& button, bool pressed)
    {
        InputEvent ev;
        ev.type = "mouse_click";
        ev.data = {{"type", "mouse_click"}, {"x", x}, {"y", y}, {"button", button}, {"pressed", pressed}};
        return ev;
    }

    // Serialize to JSON bytes for UDP transmission
    std::string serialize() const { return data.dump(); }

    // Deserialize from JSON bytes
    static bool deserialize(const std::string& json_str, InputEvent& out)
    {
        auto j = nlohmann::json::parse(json_str, nullptr, false);
        if (j.is_discarded() || !j.contains("type"))
            return false;
        out.type = j["type"].get<std::string>();
        out.data = std::move(j);
        return true;
    }
};

// Port constants
inline constexpr uint16_t INPUT_PORT = 48020;

} // namespace stream
