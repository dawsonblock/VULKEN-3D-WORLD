from dataclasses import dataclass

import numpy as np

from src.world.persistence import ChunkStore


@dataclass
class Chunk:
    position: tuple[int, int]
    voxels: np.ndarray


def test_chunk_store_roundtrip(tmp_path) -> None:
    vox = np.arange(64, dtype=np.uint8).reshape((4, 4, 4))
    chunk = Chunk(position=(0, 0), voxels=vox)

    store = ChunkStore(root=tmp_path, codec="zstd", use_rle=True, threads=1)
    store.save_chunk_sync(chunk)

    loaded = store.load_chunk(0, 0)
    assert loaded is not None
    assert np.array_equal(loaded["voxels"], vox)
