#include <gtest/gtest.h>
#include "pattern/facade.hpp"

using namespace pattern;

// ---------------------------------------------------------------------------
// Subsystem tests (direct use)
// ---------------------------------------------------------------------------
TEST(Facade, VideoFileCodecMp4)
{
    VideoFile f("movie.mp4");
    EXPECT_EQ("h264", f.codec());
}

TEST(Facade, VideoFileCodecMkv)
{
    VideoFile f("show.mkv");
    EXPECT_EQ("vp9", f.codec());
}

TEST(Facade, VideoFileCodecUnknown)
{
    VideoFile f("clip.avi");
    EXPECT_EQ("unknown", f.codec());
}

TEST(Facade, CodecFactoryExtractContainsCodecName)
{
    VideoFile f("film.mp4");
    std::string result = CodecFactory::extract(f);
    EXPECT_NE(std::string::npos, result.find("h264"));
}

TEST(Facade, BitrateReaderReadContainsPath)
{
    VideoFile f("sample.mp4");
    std::string data = BitrateReader::read(f, "h264");
    EXPECT_NE(std::string::npos, data.find("sample.mp4"));
}

TEST(Facade, BitrateReaderConvertContainsOriginalData)
{
    std::string conv = BitrateReader::convert("RAW_DATA", "vp9");
    EXPECT_NE(std::string::npos, conv.find("RAW_DATA"));
}

TEST(Facade, AudioMixerFixContainsInput)
{
    std::string mixed = AudioMixer::fix("VIDEO_DATA");
    EXPECT_NE(std::string::npos, mixed.find("VIDEO_DATA"));
}

// ---------------------------------------------------------------------------
// Facade tests
// ---------------------------------------------------------------------------
TEST(Facade, ConvertReturnsNonEmptyString)
{
    VideoConverter vc;
    std::string result = vc.convert("input.mp4", "mkv");
    EXPECT_FALSE(result.empty());
}

TEST(Facade, ConvertStoresLastResult)
{
    VideoConverter vc;
    std::string result = vc.convert("clip.mkv", "mp4");
    EXPECT_EQ(result, vc.lastResult());
}

TEST(Facade, ConvertContainsMixedAudio)
{
    VideoConverter vc;
    std::string result = vc.convert("video.mp4", "mkv");
    EXPECT_NE(std::string::npos, result.find("MixedAudio"));
}

TEST(Facade, ConvertMultipleFiles)
{
    VideoConverter vc;
    auto r1 = vc.convert("a.mp4", "mkv");
    auto r2 = vc.convert("b.mkv", "mp4");
    EXPECT_NE(r1, r2);
    EXPECT_EQ(r2, vc.lastResult());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
