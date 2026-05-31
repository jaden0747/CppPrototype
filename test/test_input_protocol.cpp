#include "stream/input_protocol.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// ---------------------------------------------------------------------------
// InputEvent factory + serialization
// ---------------------------------------------------------------------------

TEST(InputProtocol, KeyDownSerializeDeserialize)
{
    auto event = stream::InputEvent::from_key("key_down", "a");
    EXPECT_EQ(event.type, "key_down");

    std::string json = event.serialize();
    EXPECT_FALSE(json.empty());

    stream::InputEvent parsed;
    ASSERT_TRUE(stream::InputEvent::deserialize(json, parsed));
    EXPECT_EQ(parsed.type, "key_down");
    EXPECT_EQ(parsed.data["key"].get<std::string>(), "a");
}

TEST(InputProtocol, KeyUpSerializeDeserialize)
{
    auto event = stream::InputEvent::from_key("key_up", "Key.space");
    EXPECT_EQ(event.type, "key_up");

    std::string json = event.serialize();
    stream::InputEvent parsed;
    ASSERT_TRUE(stream::InputEvent::deserialize(json, parsed));
    EXPECT_EQ(parsed.type, "key_up");
    EXPECT_EQ(parsed.data["key"].get<std::string>(), "Key.space");
}

TEST(InputProtocol, MouseMoveSerializeDeserialize)
{
    auto event = stream::InputEvent::from_mouse_move(100, 200);
    EXPECT_EQ(event.type, "mouse_move");

    std::string json = event.serialize();
    stream::InputEvent parsed;
    ASSERT_TRUE(stream::InputEvent::deserialize(json, parsed));
    EXPECT_EQ(parsed.type, "mouse_move");
    EXPECT_EQ(parsed.data["x"].get<int>(), 100);
    EXPECT_EQ(parsed.data["y"].get<int>(), 200);
}

TEST(InputProtocol, MouseClickSerializeDeserialize)
{
    auto event = stream::InputEvent::from_mouse_click(50, 75, "right", true);
    EXPECT_EQ(event.type, "mouse_click");

    std::string json = event.serialize();
    stream::InputEvent parsed;
    ASSERT_TRUE(stream::InputEvent::deserialize(json, parsed));
    EXPECT_EQ(parsed.type, "mouse_click");
    EXPECT_EQ(parsed.data["x"].get<int>(), 50);
    EXPECT_EQ(parsed.data["y"].get<int>(), 75);
    EXPECT_EQ(parsed.data["button"].get<std::string>(), "right");
    EXPECT_TRUE(parsed.data["pressed"].get<bool>());
}

TEST(InputProtocol, DeserializeRejectsInvalidJson)
{
    stream::InputEvent parsed;
    EXPECT_FALSE(stream::InputEvent::deserialize("not json", parsed));
    EXPECT_FALSE(stream::InputEvent::deserialize("{}", parsed)); // no "type" field
    EXPECT_FALSE(stream::InputEvent::deserialize("", parsed));
}

// ---------------------------------------------------------------------------
// Typed event structs
// ---------------------------------------------------------------------------

TEST(InputProtocol, KeyEventJsonRoundTrip)
{
    stream::KeyEvent ev;
    ev.type = "key_down";
    ev.key  = "Key.enter";

    nlohmann::json j;
    to_json(j, ev);

    stream::KeyEvent decoded;
    from_json(j, decoded);
    EXPECT_EQ(decoded.type, "key_down");
    EXPECT_EQ(decoded.key, "Key.enter");
}

TEST(InputProtocol, MouseMoveEventJsonRoundTrip)
{
    stream::MouseMoveEvent ev;
    ev.x = 1920;
    ev.y = 1080;

    nlohmann::json j;
    to_json(j, ev);

    stream::MouseMoveEvent decoded;
    from_json(j, decoded);
    EXPECT_EQ(decoded.type, "mouse_move");
    EXPECT_EQ(decoded.x, 1920);
    EXPECT_EQ(decoded.y, 1080);
}

TEST(InputProtocol, MouseClickEventJsonRoundTrip)
{
    stream::MouseClickEvent ev;
    ev.x       = 640;
    ev.y       = 480;
    ev.button  = "middle";
    ev.pressed = false;

    nlohmann::json j;
    to_json(j, ev);

    stream::MouseClickEvent decoded;
    from_json(j, decoded);
    EXPECT_EQ(decoded.type, "mouse_click");
    EXPECT_EQ(decoded.x, 640);
    EXPECT_EQ(decoded.y, 480);
    EXPECT_EQ(decoded.button, "middle");
    EXPECT_FALSE(decoded.pressed);
}

// ---------------------------------------------------------------------------
// EventType enum helpers
// ---------------------------------------------------------------------------

TEST(InputProtocol, EventTypeToString)
{
    EXPECT_STREQ(stream::to_string(stream::InputEventType::KeyDown), "key_down");
    EXPECT_STREQ(stream::to_string(stream::InputEventType::KeyUp), "key_up");
    EXPECT_STREQ(stream::to_string(stream::InputEventType::MouseMove), "mouse_move");
    EXPECT_STREQ(stream::to_string(stream::InputEventType::MouseClick), "mouse_click");
}

TEST(InputProtocol, EventTypeFromString)
{
    EXPECT_EQ(stream::input_event_type_from_string("key_down"), stream::InputEventType::KeyDown);
    EXPECT_EQ(stream::input_event_type_from_string("key_up"), stream::InputEventType::KeyUp);
    EXPECT_EQ(stream::input_event_type_from_string("mouse_move"), stream::InputEventType::MouseMove);
    EXPECT_EQ(stream::input_event_type_from_string("mouse_click"), stream::InputEventType::MouseClick);
}
