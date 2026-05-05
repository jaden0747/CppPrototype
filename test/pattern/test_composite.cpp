#include "pattern/composite.hpp"
#include <gtest/gtest.h>

using namespace pattern;

// ---------------------------------------------------------------------------
// Leaf (File) tests
// ---------------------------------------------------------------------------
TEST(Composite, FileReturnsItsSize)
{
    File f("doc.txt", 1024);
    EXPECT_EQ(1024LL, f.size());
}

TEST(Composite, FileIsNotDirectory)
{
    File f("img.png", 512);
    EXPECT_FALSE(f.isDirectory());
}

TEST(Composite, FileHasCorrectName)
{
    File f("readme.md", 256);
    EXPECT_EQ("readme.md", f.name());
}

// ---------------------------------------------------------------------------
// Composite (Directory) tests
// ---------------------------------------------------------------------------
TEST(Composite, EmptyDirectorySizeIsZero)
{
    Directory d("empty");
    EXPECT_EQ(0LL, d.size());
}

TEST(Composite, DirectoryIsDirectory)
{
    Directory d("home");
    EXPECT_TRUE(d.isDirectory());
}

TEST(Composite, DirectorySizeIsChildSum)
{
    Directory d("root");
    d.add(std::unique_ptr<FileSystemNode>(new File("a.txt", 100)));
    d.add(std::unique_ptr<FileSystemNode>(new File("b.txt", 200)));
    EXPECT_EQ(300LL, d.size());
}

TEST(Composite, NestedDirectorySizes)
{
    auto inner = std::unique_ptr<Directory>(new Directory("inner"));
    inner->add(std::unique_ptr<FileSystemNode>(new File("x.cpp", 500)));
    inner->add(std::unique_ptr<FileSystemNode>(new File("y.cpp", 300)));

    Directory outer("outer");
    outer.add(std::unique_ptr<FileSystemNode>(new File("main.cpp", 200)));
    outer.add(std::move(inner));

    EXPECT_EQ(1000LL, outer.size());
}

TEST(Composite, ChildCountCorrect)
{
    Directory d("src");
    d.add(std::unique_ptr<FileSystemNode>(new File("a.cpp", 10)));
    d.add(std::unique_ptr<FileSystemNode>(new File("b.cpp", 20)));
    EXPECT_EQ(2u, d.childCount());
}

TEST(Composite, ChildAtReturnsCorrectNode)
{
    Directory d("src");
    d.add(std::unique_ptr<FileSystemNode>(new File("first.cpp", 10)));
    EXPECT_EQ("first.cpp", d.childAt(0)->name());
}

TEST(Composite, ChildAtOutOfRangeThrows)
{
    Directory d("src");
    EXPECT_THROW(d.childAt(0), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Deep nesting
// ---------------------------------------------------------------------------
TEST(Composite, DeepNestedSizes)
{
    // root/a/b/c/file.txt = 999
    auto c = std::unique_ptr<Directory>(new Directory("c"));
    c->add(std::unique_ptr<FileSystemNode>(new File("file.txt", 999)));

    auto b = std::unique_ptr<Directory>(new Directory("b"));
    b->add(std::move(c));

    auto a = std::unique_ptr<Directory>(new Directory("a"));
    a->add(std::move(b));

    Directory root("root");
    root.add(std::move(a));

    EXPECT_EQ(999LL, root.size());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
