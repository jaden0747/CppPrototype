"""
Facade Pattern
==============
Intent: Provide a simplified interface to a complex subsystem.

Real-world analogy: A video converter facade hides codec detection, bitrate
reading, conversion, and audio mixing behind a single `convert()` call.
"""

import os


# ---------------------------------------------------------------------------
# Subsystem classes
# ---------------------------------------------------------------------------

class VideoFile:
    def __init__(self, path: str):
        self.path = path

    def codec(self) -> str:
        ext = os.path.splitext(self.path)[1].lower().lstrip(".")
        return {"mp4": "h264", "mkv": "vp9"}.get(ext, "unknown")


class CodecFactory:
    @staticmethod
    def extract(file: VideoFile) -> str:
        return f"Codec[{file.codec()}]"


class BitrateReader:
    @staticmethod
    def read(file: VideoFile, codec: str) -> str:
        return f"Bitrate for {file.path} using {codec}"

    @staticmethod
    def convert(data: str, codec: str) -> str:
        return f"Converted({data}, {codec})"


class AudioMixer:
    @staticmethod
    def fix(data: str) -> str:
        return f"MixedAudio({data})"


# ---------------------------------------------------------------------------
# Facade
# ---------------------------------------------------------------------------

class VideoConverter:
    def __init__(self):
        self._last_result: str = ""

    def convert(self, file_path: str, target_format: str) -> str:
        file         = VideoFile(file_path)
        source_codec = CodecFactory.extract(file)
        target_codec = CodecFactory.extract(VideoFile(f"output.{target_format}"))
        data         = BitrateReader.read(file, source_codec)
        converted    = BitrateReader.convert(data, target_codec)
        result       = AudioMixer.fix(converted)
        self._last_result = result
        return result

    @property
    def last_result(self) -> str:
        return self._last_result


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    vc = VideoConverter()
    result = vc.convert("holiday_video.mp4", "mkv")
    print(result)
