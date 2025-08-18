"""Basic smoke test ensuring the C++ toolchain can build a shared library."""

from __future__ import annotations

import ctypes
import subprocess
from pathlib import Path


def test_build_simple_shared_library(tmp_path: Path) -> None:
    """Compile and load a trivial C++ library at runtime."""

    cpp = tmp_path / "chunk_store.cpp"
    cpp.write_text(
        """
        extern "C" int add(int a, int b) { return a + b; }
        """
    )

    lib = tmp_path / "libchunk_store.so"
    subprocess.run(
        [
            "g++",
            "-std=c++17",
            "-shared",
            "-fPIC",
            str(cpp),
            "-o",
            str(lib),
        ],
        check=True,
    )

    dll = ctypes.CDLL(str(lib))
    assert dll.add(2, 3) == 5

