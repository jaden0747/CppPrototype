#include <gtest/gtest.h>
#include "pattern/bridge.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// TV tests
// ---------------------------------------------------------------------------
TEST(Bridge, TVStartsDisabled)
{
    TV tv;
    EXPECT_FALSE(tv.isEnabled());
}

TEST(Bridge, TVEnableDisable)
{
    TV tv;
    tv.enable();
    EXPECT_TRUE(tv.isEnabled());
    tv.disable();
    EXPECT_FALSE(tv.isEnabled());
}

TEST(Bridge, TVVolumeClampedAt100)
{
    TV tv;
    tv.setVolume(150);
    EXPECT_EQ(100, tv.volume());
}

TEST(Bridge, TVVolumeClampedAt0)
{
    TV tv;
    tv.setVolume(-10);
    EXPECT_EQ(0, tv.volume());
}

// ---------------------------------------------------------------------------
// Remote + TV
// ---------------------------------------------------------------------------
TEST(Bridge, TogglePowerEnablesTV)
{
    RemoteControl remote(std::unique_ptr<Device>(new TV()));
    remote.togglePower();
    EXPECT_TRUE(remote.device().isEnabled());
}

TEST(Bridge, TogglePowerTwiceRestoresState)
{
    RemoteControl remote(std::unique_ptr<Device>(new TV()));
    remote.togglePower();
    remote.togglePower();
    EXPECT_FALSE(remote.device().isEnabled());
}

TEST(Bridge, VolumeUpIncreasesBy10)
{
    RemoteControl remote(std::unique_ptr<Device>(new TV()));
    int initial = remote.device().volume();
    remote.volumeUp();
    EXPECT_EQ(initial + 10, remote.device().volume());
}

TEST(Bridge, VolumeDownDecreasesBy10)
{
    RemoteControl remote(std::unique_ptr<Device>(new TV()));
    int initial = remote.device().volume();
    remote.volumeDown();
    EXPECT_EQ(initial - 10, remote.device().volume());
}

TEST(Bridge, ChannelUpIncreasesBy1)
{
    RemoteControl remote(std::unique_ptr<Device>(new TV()));
    int initial = remote.device().channel();
    remote.channelUp();
    EXPECT_EQ(initial + 1, remote.device().channel());
}

// ---------------------------------------------------------------------------
// Remote + Radio (same abstraction, different implementation)
// ---------------------------------------------------------------------------
TEST(Bridge, RemoteWorksWithRadio)
{
    RemoteControl remote(std::unique_ptr<Device>(new Radio()));
    remote.togglePower();
    EXPECT_TRUE(remote.device().isEnabled());
    EXPECT_EQ("Radio", remote.device().name());
}

// ---------------------------------------------------------------------------
// AdvancedRemote
// ---------------------------------------------------------------------------
TEST(Bridge, AdvancedRemoteMuteSetsVolumeToZero)
{
    AdvancedRemote remote(std::unique_ptr<Device>(new TV()));
    remote.mute();
    EXPECT_EQ(0, remote.device().volume());
}

TEST(Bridge, AdvancedRemoteSetChannel)
{
    AdvancedRemote remote(std::unique_ptr<Device>(new TV()));
    remote.setChannel(42);
    EXPECT_EQ(42, remote.device().channel());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
