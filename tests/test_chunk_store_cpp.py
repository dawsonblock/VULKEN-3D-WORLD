
# mypy: ignore-errors

import os
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path

import pytest
np = pytest.importorskip("numpy")

from src.world.persistence import ChunkStore as PyChunkStore


def _have_cpp_toolchain() -> bool:
    if shutil.which("g++") is None:
        return False
    code = "#include <zstd.h>\nint main(){return 0;}"
    with tempfile.NamedTemporaryFile("w", suffix=".cpp") as src:
        src.write(code)
        src.flush()
        try:
            subprocess.check_output(
                ["g++", src.name, "-lzstd", "-o", os.devnull],
                stderr=subprocess.STDOUT,
            )
        except Exception:
            return False
    return True


pytestmark = pytest.mark.skipif(
    not _have_cpp_toolchain(),
    reason="g++ with zstd headers required",
)


def _compile_loader(tmp_path: Path, repo_root: Path) -> Path:
    code = r"""
#include "src/world/persistence.hpp"
#include <iostream>
using namespace world;
int main(int argc, char** argv){
    std::string root = argv[1];
    ChunkStore store(root, "zstd", true);
    auto chunk = store.load_chunk(0,0);
    if(!chunk) return 1;
    std::cout.write(reinterpret_cast<const char*>(chunk->voxels.data()), chunk->voxels.size());
    return 0;
}
"""
    src_file = tmp_path / "loader.cpp"
    src_file.write_text(code)
    exe = tmp_path / "loader"
    subprocess.check_call([
        "g++", "-std=c++20", str(src_file), str(repo_root / "src/world/persistence.cpp"),
        "-I", str(repo_root), "-lzstd", "-pthread", "-o", str(exe)
    ], cwd=repo_root)
    return exe


def test_cpp_chunk_store_roundtrip(tmp_path: Path) -> None:
    repo_root = Path(__file__).resolve().parent.parent
    vox = np.arange(64, dtype=np.uint8).reshape((4, 4, 4))

    @dataclass
    class Chunk:
        position: tuple[int, int]
        voxels: np.ndarray

    chunk = Chunk(position=(0, 0), voxels=vox)

    store = PyChunkStore(root=tmp_path, codec="zstd", use_rle=True, threads=1)
    store.save_chunk_sync(chunk)

    loader = _compile_loader(tmp_path, repo_root)
    output = subprocess.check_output([str(loader), str(tmp_path)], cwd=repo_root)
    assert output == vox.tobytes()

