"""Tests for the Facade pattern."""

import sys
import os
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'pattern'))
from facade import VideoFile, CodecFactory, BitrateReader, AudioMixer, VideoConverter


class TestFacade(unittest.TestCase):

    # ------------------------------------------------------------------
    # Subsystem (direct use)
    # ------------------------------------------------------------------
    def test_video_file_codec_mp4(self):
        self.assertEqual("h264", VideoFile("movie.mp4").codec())

    def test_video_file_codec_mkv(self):
        self.assertEqual("vp9", VideoFile("show.mkv").codec())

    def test_codec_factory_contains_codec(self):
        result = CodecFactory.extract(VideoFile("film.mp4"))
        self.assertIn("h264", result)

    def test_bitrate_reader_contains_path(self):
        data = BitrateReader.read(VideoFile("sample.mp4"), "h264")
        self.assertIn("sample.mp4", data)

    def test_bitrate_reader_convert_contains_data(self):
        conv = BitrateReader.convert("RAW_DATA", "vp9")
        self.assertIn("RAW_DATA", conv)

    def test_audio_mixer_contains_input(self):
        self.assertIn("VIDEO_DATA", AudioMixer.fix("VIDEO_DATA"))

    # ------------------------------------------------------------------
    # Facade
    # ------------------------------------------------------------------
    def test_convert_returns_non_empty(self):
        self.assertTrue(len(VideoConverter().convert("input.mp4", "mkv")) > 0)

    def test_convert_stores_last_result(self):
        vc = VideoConverter()
        result = vc.convert("clip.mkv", "mp4")
        self.assertEqual(result, vc.last_result)

    def test_convert_contains_mixed_audio(self):
        self.assertIn("MixedAudio", VideoConverter().convert("video.mp4", "mkv"))

    def test_convert_multiple_files_different_results(self):
        vc = VideoConverter()
        r1 = vc.convert("a.mp4", "mkv")
        r2 = vc.convert("b.mkv", "mp4")
        self.assertNotEqual(r1, r2)
        self.assertEqual(r2, vc.last_result)


if __name__ == "__main__":
    unittest.main()
