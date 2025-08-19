import json
from dataclasses import dataclass
from pathlib import Path

import numpy as np

from src.world.persistence import ChunkStore


@dataclass
class Chunk:
    position: tuple[int, int]
    voxels: np.ndarray


def test_manifest_and_compaction(tmp_path: Path) -> None:
    vox = np.zeros((4, 4, 4), dtype=np.uint8)
    chunk = Chunk(position=(0, 0), voxels=vox)
    store = ChunkStore(root=tmp_path, codec="zstd", use_rle=True, threads=1)

    store.save_chunk_sync(chunk)
    manifest = json.loads((tmp_path / "manifest.json").read_text())
    assert "r.0.0" in manifest
    reg = manifest["r.0.0"]
    assert reg["codec"] == "zstd"
    assert reg["chunks"]["0,0"]["offset"] == 0

    idx = tmp_path / "r.0.0" / "index.json"
    table = json.loads(idx.read_text())
    assert "0,0" in table

    stray = tmp_path / "r.0.0" / "c.99.99.bin"
    stray.write_bytes(b"garbage")
    store.compact()
    assert not stray.exists()

    store.delete_chunk(0, 0)
    assert not idx.exists()
    assert not (tmp_path / "r.0.0").exists()
    manifest = json.loads((tmp_path / "manifest.json").read_text())
    assert "r.0.0" not in manifest
