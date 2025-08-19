# mypy: ignore-errors

import subprocess
from pathlib import Path

import pytest


def _compile_roundtrip(tmp_dir: Path, repo_root: Path) -> Path:
    code = r"""
#include "src/world/persistence.hpp"
#include <iostream>
using namespace world;
int main(int argc, char** argv){
    std::string root = argv[1];
    bool use_rle = std::stoi(argv[2]) != 0;
    ChunkStore store(root, "zstd", use_rle);
    Chunk chunk;
    chunk.position = {0,0};
    chunk.height = 4;
    chunk.size = 4;
    chunk.voxels.resize(64);
    for(int i=0;i<64;i++) chunk.voxels[i] = static_cast<std::uint8_t>(i);
    store.save_chunk_sync(chunk);
    auto loaded = store.load_chunk(0,0);
    if(!loaded) return 1;
    std::cout.write(reinterpret_cast<const char*>(loaded->voxels.data()), loaded->voxels.size());
    return 0;
}
"""
    src_file = tmp_dir / "roundtrip.cpp"
    src_file.write_text(code)
    exe = tmp_dir / "roundtrip"
    subprocess.check_call([
        "g++", "-std=c++20", str(src_file), str(repo_root / "src/world/persistence.cpp"),
        "-I", str(repo_root), "-lzstd", "-pthread", "-o", str(exe)
    ], cwd=repo_root)
    return exe


@pytest.mark.parametrize("use_rle", [True, False])
def test_cpp_chunk_store_roundtrip(tmp_path: Path, use_rle: bool) -> None:
    repo_root = Path(__file__).resolve().parent.parent
    exe = _compile_roundtrip(tmp_path, repo_root)
    output = subprocess.check_output([str(exe), str(tmp_path), "1" if use_rle else "0"], cwd=repo_root)
    assert output == bytes(range(64))

