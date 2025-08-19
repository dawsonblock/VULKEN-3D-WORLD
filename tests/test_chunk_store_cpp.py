# mypy: ignore-errors

import json
import json
import subprocess
from dataclasses import dataclass
from pathlib import Path

import pytest

np = pytest.importorskip("numpy")

from src.world.persistence import ChunkStore as PyChunkStore


def _compile_loader(tmp_path: Path, repo_root: Path, codec: str) -> Path:
    code = f"""
#include \"src/world/persistence.hpp\"
#include <iostream>
using namespace world;
int main(int argc, char** argv){{
    std::string root = argv[1];
    ChunkStore store(root, \"{codec}\", true);
    auto chunk = store.load_chunk(0,0);
    if(!chunk) return 1;
    std::cout.write(reinterpret_cast<const char*>(chunk->voxels.data()), chunk->voxels.size());
    return 0;
}}
"""
    src_file = tmp_path / "loader.cpp"
    src_file.write_text(code)
    exe = tmp_path / f"loader_{codec}"
    subprocess.check_call([
        "g++", "-std=c++20", str(src_file), str(repo_root / "src/world/persistence.cpp"),
        "-I", str(repo_root), "-lzstd", "-llz4", "-pthread", "-o", str(exe)
    ], cwd=repo_root)
    return exe


def _compile_compactor(tmp_path: Path, repo_root: Path, codec: str) -> Path:
    code = f"""
#include \"src/world/persistence.hpp\"
using namespace world;
int main(int argc, char** argv){{
    std::string root = argv[1];
    ChunkStore store(root, \"{codec}\", true);
    store.compact_region(0,0);
    return 0;
}}
"""
    src_file = tmp_path / "compactor.cpp"
    src_file.write_text(code)
    exe = tmp_path / f"compactor_{codec}"
    subprocess.check_call([
        "g++", "-std=c++20", str(src_file), str(repo_root / "src/world/persistence.cpp"),
        "-I", str(repo_root), "-lzstd", "-llz4", "-pthread", "-o", str(exe)
    ], cwd=repo_root)
    return exe


@pytest.mark.parametrize("codec", ["zstd", "lz4"])
def test_cpp_chunk_store_roundtrip(tmp_path: Path, codec: str) -> None:
    repo_root = Path(__file__).resolve().parent.parent
    vox = np.arange(64, dtype=np.uint8).reshape((4, 4, 4))

    @dataclass
    class Chunk:
        position: tuple[int, int]
        voxels: np.ndarray

    chunk = Chunk(position=(0, 0), voxels=vox)

    store = PyChunkStore(root=tmp_path, codec=codec, use_rle=True, threads=1)
    store.save_chunk_sync(chunk)

    loader = _compile_loader(tmp_path, repo_root, codec)
    output = subprocess.check_output([str(loader), str(tmp_path)], cwd=repo_root)
    assert output == vox.tobytes()


def test_region_manifest_compaction(tmp_path: Path) -> None:
    repo_root = Path(__file__).resolve().parent.parent
    vox = np.zeros((4, 4, 4), dtype=np.uint8)

    @dataclass
    class Chunk:
        position: tuple[int, int]
        voxels: np.ndarray

    chunk = Chunk(position=(0, 0), voxels=vox)

    store = PyChunkStore(root=tmp_path, codec="zstd", use_rle=True, threads=1)
    store.save_chunk_sync(chunk)

    manifest = tmp_path / "r.0.0" / "region_manifest.json"
    table = json.loads(manifest.read_text())
    assert "0,0" in table

    # remove chunk file then compact via C++ helper
    (tmp_path / "r.0.0" / "c.0.0.bin").unlink()
    compactor = _compile_compactor(tmp_path, repo_root, "zstd")
    subprocess.check_call([str(compactor), str(tmp_path)], cwd=repo_root)

    table = json.loads(manifest.read_text())
    assert "0,0" not in table
