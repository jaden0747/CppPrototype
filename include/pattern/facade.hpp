#pragma once
#include <stdexcept>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Facade Pattern
//
// Intent: Provide a simplified interface to a complex subsystem. The Facade
// doesn't hide the subsystem — clients can still use it directly — but it
// gives a convenient shorthand for the most common use cases.
//
// Real-world analogy: Starting a car. Under the hood: fuel injection, ignition
// timing, battery check, transmission initialisation… The driver just turns
// the key. The key is the Facade.
//
// Key C++ mechanics used:
//  - Facade class owns (or references) subsystem objects.
//  - High-level methods orchestrate multiple subsystem calls.
//  - Subsystem classes are fully usable without the Facade.
//
// When to use:
//  - You want to provide a simple interface to a complex subsystem.
//  - You want to layer your subsystem (low-level → high-level API).
//  - Client code has too many dependencies on subsystem internals.
// ---------------------------------------------------------------------------

namespace pattern
{

// ---------- Subsystem classes ---------------------------------------------

class VideoFile
{
public:
    explicit VideoFile(const std::string& path)
        : path_(path)
    {
    }
    const std::string& path() const
    {
        return path_;
    }
    std::string codec() const
    {
        if (path_.size() >= 3 && path_.substr(path_.size() - 3) == "mp4")
            return "h264";
        if (path_.size() >= 3 && path_.substr(path_.size() - 3) == "mkv")
            return "vp9";
        return "unknown";
    }

private:
    std::string path_;
};

class CodecFactory
{
public:
    static std::string extract(const VideoFile& file)
    {
        return "Codec[" + file.codec() + "]";
    }
};

class BitrateReader
{
public:
    static std::string read(const VideoFile& file, const std::string& codec)
    {
        return "Bitrate for " + file.path() + " using " + codec;
    }

    static std::string convert(const std::string& data, const std::string& codec)
    {
        return "Converted(" + data + ", " + codec + ")";
    }
};

class AudioMixer
{
public:
    static std::string fix(const std::string& data)
    {
        return "MixedAudio(" + data + ")";
    }
};

// ---------- Facade --------------------------------------------------------

class VideoConverter
{
public:
    std::string convert(const std::string& filePath, const std::string& targetFormat)
    {
        VideoFile file(filePath);

        std::string sourceCodec = CodecFactory::extract(file);
        std::string targetCodec = CodecFactory::extract(VideoFile("output." + targetFormat));
        std::string data        = BitrateReader::read(file, sourceCodec);
        std::string converted   = BitrateReader::convert(data, targetCodec);
        std::string result      = AudioMixer::fix(converted);

        lastResult_ = result;
        return result;
    }

    const std::string& lastResult() const
    {
        return lastResult_;
    }

private:
    std::string lastResult_;
};

} // namespace pattern
