#!/usr/bin/env python3
"""Build the diff_voxelize native extension."""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / "src" / "ai"
SRC = ROOT / "diff_voxelize.cpp"

if sys.platform.startswith("win"):
    ext = ".dll"
    extra = []
elif sys.platform == "darwin":
    ext = ".dylib"
    extra = ["-dynamiclib"]
else:
    ext = ".so"
    extra = ["-fPIC"]

LIB_PATH = ROOT / f"diff_voxelize{ext}"


def run(cmd: list[str]) -> None:
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as exc:
        output = exc.output.decode()
        raise RuntimeError(
            f"Command {' '.join(cmd)} failed with code {exc.returncode}:\n{output}"
        ) from exc


def build() -> None:
    cmd = [
        "g++",
        "-std=c++17",
        "-shared",
        str(SRC),
        "-o",
        str(LIB_PATH),
    ] + extra
    run(cmd)
    print(f"Built {LIB_PATH}")


if __name__ == "__main__":
    build()
