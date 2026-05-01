#include <gtest/gtest.h>
#include "pattern/proxy.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// 1. Virtual Proxy (lazy init)
// ---------------------------------------------------------------------------
TEST(Proxy, LazyProxyNotLoadedBeforeDisplay)
{
    LazyImageProxy proxy("photo.png");
    EXPECT_FALSE(proxy.isLoaded());
}

TEST(Proxy, LazyProxyLoadedAfterDisplay)
{
    LazyImageProxy proxy("photo.png");
    proxy.display();
    EXPECT_TRUE(proxy.isLoaded());
}

TEST(Proxy, LazyProxyNameMatchesFilename)
{
    LazyImageProxy proxy("vacation.jpg");
    EXPECT_EQ("vacation.jpg", proxy.name());
}

TEST(Proxy, LazyProxyDisplayTwiceStaysLoaded)
{
    LazyImageProxy proxy("img.bmp");
    proxy.display();
    proxy.display();
    EXPECT_TRUE(proxy.isLoaded());
}

// ---------------------------------------------------------------------------
// 2. Protection Proxy
// ---------------------------------------------------------------------------
TEST(Proxy, UserCanGetData)
{
    ProtectionProxy proxy(std::unique_ptr<Service>(new RealService()), Role::User);
    EXPECT_EQ("Sensitive data", proxy.getData());
}

TEST(Proxy, GuestCannotGetData)
{
    ProtectionProxy proxy(std::unique_ptr<Service>(new RealService()), Role::Guest);
    EXPECT_THROW(proxy.getData(), std::runtime_error);
}

TEST(Proxy, AdminCanDeleteData)
{
    ProtectionProxy proxy(std::unique_ptr<Service>(new RealService()), Role::Admin);
    EXPECT_TRUE(proxy.deleteData());
}

TEST(Proxy, UserCannotDeleteData)
{
    ProtectionProxy proxy(std::unique_ptr<Service>(new RealService()), Role::User);
    EXPECT_THROW(proxy.deleteData(), std::runtime_error);
}

TEST(Proxy, GuestCannotDeleteData)
{
    ProtectionProxy proxy(std::unique_ptr<Service>(new RealService()), Role::Guest);
    EXPECT_THROW(proxy.deleteData(), std::runtime_error);
}

// ---------------------------------------------------------------------------
// 3. Caching Proxy
// ---------------------------------------------------------------------------
TEST(Proxy, CachingProxyReturnsSameValue)
{
    auto slow = std::unique_ptr<SlowDataSource>(new SlowDataSource());
    CachingProxy proxy(std::move(slow));
    EXPECT_EQ("value_of_a", proxy.fetch("a"));
}

TEST(Proxy, CachingProxyOnlyCallsRealOnce)
{
    auto* rawSlow = new SlowDataSource();
    CachingProxy proxy{std::unique_ptr<DataSource>(rawSlow)};
    proxy.fetch("key1");
    proxy.fetch("key1");
    proxy.fetch("key1");
    EXPECT_EQ(1, rawSlow->fetchCount);
}

TEST(Proxy, CachingProxyCacheSizeGrowsForNewKeys)
{
    CachingProxy proxy(std::unique_ptr<DataSource>(new SlowDataSource()));
    proxy.fetch("k1");
    proxy.fetch("k2");
    proxy.fetch("k1");
    EXPECT_EQ(2u, proxy.cacheSize());
}

TEST(Proxy, CachingProxyDifferentKeysFetchedOnce)
{
    auto* rawSlow = new SlowDataSource();
    CachingProxy proxy{std::unique_ptr<DataSource>(rawSlow)};
    proxy.fetch("a");
    proxy.fetch("b");
    proxy.fetch("c");
    EXPECT_EQ(3, rawSlow->fetchCount);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
