from __future__ import annotations

import numpy as np

from src.world.generator import NoiseChunkGenerator
from src.world.chunk_manager import ChunkManager
from src.world.game import GameWorld


def test_noise_generator_deterministic() -> None:
    gen = NoiseChunkGenerator(seed=123, num_biomes=3)
    c1 = gen.generate(0, 0, 0)
    c2 = gen.generate(0, 0, 0)
    assert np.array_equal(c1.voxels, c2.voxels)
    assert np.array_equal(c1.biomes, c2.biomes)


def test_chunk_manager_load_and_evict() -> None:
    gen = NoiseChunkGenerator(seed=0)
    mgr = ChunkManager(gen, radius=1, max_chunks=100)
    world = GameWorld(mgr)

    world.step((0.0, 0.0, 0.0))
    assert len(mgr.chunks) == 27  # 3x3x3 around the player

    chunk = mgr.get_chunk(0, 0, 0)
    assert chunk.voxels.shape == (32, 32, 32)
    assert chunk.biomes.shape == (32, 32, 32)

    # Move player far enough to unload the original center chunk
    world.step((3 * CHUNK_SIZE, 0.0, 0.0))  # move to chunk x=3 (since (3 * CHUNK_SIZE)/CHUNK_SIZE=3)
    assert (0, 0, 0) not in mgr.chunks
    assert (3, 0, 0) in mgr.chunks
