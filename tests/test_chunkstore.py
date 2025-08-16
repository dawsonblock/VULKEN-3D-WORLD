import numpy as np
from src.world.persistence import ChunkStore

class DummyChunk:
    def __init__(self, position, voxels):
        self.position = position
        self.voxels = voxels

def test_chunkstore_save_and_load(tmp_path):
    voxels = np.arange(8, dtype=np.uint8).reshape((2, 2, 2))
    chunk = DummyChunk((0, 0), voxels)
    store = ChunkStore(root=tmp_path, use_rle=True)
    store.save_chunk_sync(chunk)
    loaded = store.load_chunk(0, 0)
    assert loaded is not None
    assert np.array_equal(loaded["voxels"], voxels)
    assert loaded["height"] == voxels.shape[1]
    assert loaded["size"] == voxels.shape[2]
