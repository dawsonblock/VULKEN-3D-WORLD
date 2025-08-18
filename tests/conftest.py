import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIB_PATH = Path(__file__).with_name("physics_cpp.so")

# Ensure the project's src directory is importable
sys.path.append(str(ROOT))

# Build the small C++ helper used by a few tests.  The build is attempted
# once when the test suite starts; if the compilation fails the relevant
# tests will gracefully skip when they try to load the library.
if not LIB_PATH.exists():  # pragma: no cover - exercised in CI
    try:
        subprocess.check_call(
            [
                "g++",
                "-std=c++17",
                "-shared",
                "-fPIC",
                str(ROOT / "src/physics_cpp/physics_c_api.cpp"),
                "-I",
                str(ROOT / "src/physics_cpp"),
                "-o",
                str(LIB_PATH),
            ]
        )
    except (subprocess.CalledProcessError, OSError, FileNotFoundError):
        # The tests that rely on this library will skip if it is missing.
        pass
