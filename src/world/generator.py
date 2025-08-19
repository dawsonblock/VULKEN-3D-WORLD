"""Procedural chunk generation using simple noise."""
from __future__ import annotations

import numpy as np

from .chunk import Chunk, CHUNK_SIZE


class NoiseChunkGenerator:
    """Generate voxel chunks filled using deterministic random noise.

    The generator produces ``CHUNK_SIZE`` cubed voxel arrays where each
    value represents a block id. A corresponding biome id array is also
    produced. Both arrays are deterministic for a given chunk coordinate
    and seed.
    """

    def __init__(self, seed: int = 0, num_biomes: int = 4):
        self.seed = seed
        self.num_biomes = num_biomes

    def _rng_for(self, cx: int, cy: int, cz: int) -> np.random.Generator:
        """Return a seeded RNG unique to the chunk coordinates."""

        s = (cx * 73856093) ^ (cy * 19349663) ^ (cz * 83492791) ^ self.seed
        return np.random.default_rng(s & 0xFFFFFFFF)

    def generate(self, cx: int, cy: int, cz: int) -> Chunk:
        """Generate a chunk at the given coordinates."""

        rng = self._rng_for(cx, cy, cz)
        noise = rng.random((CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE))
        voxels = (noise > 0.5).astype(np.uint8)
        biomes = np.floor(noise * self.num_biomes).astype(np.uint8)
        return Chunk(position=(cx, cy, cz), voxels=voxels, biomes=biomes)


__all__ = ["NoiseChunkGenerator"]
