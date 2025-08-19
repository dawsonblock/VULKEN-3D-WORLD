import numpy as np

from src.world.chunk_cache import CHUNK_SIZE, ChunkCache


def _loader(pos, lod):
    size = CHUNK_SIZE >> lod
    return np.ones((size, size, size), dtype=np.uint8)


def test_chunk_cache_lod_and_eviction() -> None:
    cache = ChunkCache(capacity=2, lod_distances=(32.0,))
    player = (0.0, 0.0, 0.0)

    cache.get_chunk((0, 0, 0), player, _loader)
    cache.get_chunk((1, 0, 0), player, _loader)
    assert len(cache._cache) == 2

    cache.get_chunk((2, 0, 0), player, _loader)
    assert len(cache._cache) == 2  # LRU eviction

    far_player = (100.0, 0.0, 0.0)
    chunk = cache.get_chunk((10, 0, 0), far_player, _loader)
    assert chunk.lod > 0

