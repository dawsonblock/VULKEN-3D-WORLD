import importlib
import sys
from unittest import mock


def _load(platform: str):
    with mock.patch.object(sys, "platform", platform), \
         mock.patch("pathlib.Path.exists", return_value=True), \
         mock.patch("ctypes.CDLL"):
        sys.modules.pop("src.ai.diff_voxelize", None)
        return importlib.import_module("src.ai.diff_voxelize")


def test_linux_platform():
    mod = _load("linux")
    assert mod.LIB_PATH.name == "diff_voxelize.so"


def test_linux2_platform():
    mod = _load("linux2")
    assert mod.LIB_PATH.name == "diff_voxelize.so"


def test_darwin_platform():
    mod = _load("darwin")
    assert mod.LIB_PATH.name == "diff_voxelize.dylib"


def test_win32_platform():
    mod = _load("win32")
    assert mod.LIB_PATH.name == "diff_voxelize.dll"
