#include "pattern/mediator.hpp"
#include <gtest/gtest.h>

using namespace pattern;

struct ChatFixture
{
    ChatRoom room;
    ChatUser alice{"Alice"};
    ChatUser bob{"Bob"};
    ChatUser carol{"Carol"};

    ChatFixture()
    {
        room.addUser(&alice);
        room.addUser(&bob);
        room.addUser(&carol);
    }
};

// ---------------------------------------------------------------------------
// Basic message delivery
// ---------------------------------------------------------------------------
TEST(Mediator, SenderDoesNotReceiveOwnMessage)
{
    ChatFixture f;
    f.alice.send("Hello everyone");
    EXPECT_TRUE(f.alice.inbox().empty());
}

TEST(Mediator, OtherUsersReceiveMessage)
{
    ChatFixture f;
    f.alice.send("Hello");
    EXPECT_EQ(1u, f.bob.inbox().size());
    EXPECT_EQ(1u, f.carol.inbox().size());
}

TEST(Mediator, InboxContainsSenderName)
{
    ChatFixture f;
    f.alice.send("Hi");
    ASSERT_FALSE(f.bob.inbox().empty());
    EXPECT_NE(std::string::npos, f.bob.inbox()[0].find("Alice"));
}

TEST(Mediator, InboxContainsMessageText)
{
    ChatFixture f;
    f.alice.send("secret_msg");
    ASSERT_FALSE(f.bob.inbox().empty());
    EXPECT_NE(std::string::npos, f.bob.inbox()[0].find("secret_msg"));
}

// ---------------------------------------------------------------------------
// Multiple senders
// ---------------------------------------------------------------------------
TEST(Mediator, BobSendsToAliceAndCarol)
{
    ChatFixture f;
    f.bob.send("Hey");
    EXPECT_EQ(1u, f.alice.inbox().size());
    EXPECT_TRUE(f.bob.inbox().empty());
    EXPECT_EQ(1u, f.carol.inbox().size());
}

TEST(Mediator, MultipleMessages)
{
    ChatFixture f;
    f.alice.send("msg1");
    f.alice.send("msg2");
    EXPECT_EQ(2u, f.bob.inbox().size());
}

// ---------------------------------------------------------------------------
// ChatRoom user count
// ---------------------------------------------------------------------------
TEST(Mediator, ChatRoomUserCount)
{
    ChatRoom room;
    ChatUser u1{"U1"}, u2{"U2"};
    room.addUser(&u1);
    room.addUser(&u2);
    EXPECT_EQ(2u, room.userCount());
}

// ---------------------------------------------------------------------------
// Component without mediator: no crash (trigger does nothing)
// ---------------------------------------------------------------------------
TEST(Mediator, ComponentWithoutMediatorDoesNotCrash)
{
    ChatUser loner{"Loner"};
    EXPECT_NO_THROW(loner.send("hello"));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
