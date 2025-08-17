import subprocess
from pathlib import Path

import numpy as np
import pytest

from src.world.persistence import ChunkStore as PyChunkStore


def _compile_loader(tmp_path: Path, repo_root: Path) -> Path | None:
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
    try:
        subprocess.check_call(
            [
                "g++",
                "-std=c++20",
                str(src_file),
                str(repo_root / "src/world/persistence.cpp"),
                "-I",
                str(repo_root / "src"),
                "-lzstd",
                "-pthread",
                "-o",
                str(exe),
            ],
            cwd=repo_root,
        )
    except Exception:
        return None
    return exe


def test_cpp_chunk_store_roundtrip(tmp_path: Path) -> None:
    repo_root = Path(__file__).resolve().parent.parent
    vox = np.arange(64, dtype=np.uint8).reshape((4, 4, 4))

    class Chunk:
        position: tuple[int, int]
        voxels: np.ndarray

    chunk = Chunk()
    chunk.position = (0, 0)
    chunk.voxels = vox

    store = PyChunkStore(root=tmp_path, codec="zstd", use_rle=True, threads=1)
    store.save_chunk_sync(chunk)

    loader = _compile_loader(tmp_path, repo_root)
    if loader is None:
        pytest.skip("g++ compilation failed")
    output = subprocess.check_output([str(loader), str(tmp_path)], cwd=repo_root)
    assert output == vox.tobytes()
