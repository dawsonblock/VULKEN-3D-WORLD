"""Chunk streaming utilities with LRU caching and distance-based LOD."""

from __future__ import annotations

from collections import OrderedDict
from dataclasses import dataclass
from math import sqrt
from typing import Callable, Dict, Iterable, Tuple

import numpy as np

from .greedy_mesher import MeshBuffers, greedy_mesh

CHUNK_SIZE = 32


@dataclass
class Chunk:
    position: Tuple[int, int, int]
    voxels: np.ndarray
    lod: int
    mesh: MeshBuffers | None = None


class ChunkCache:
    """LRU cache for voxel chunks.

    Parameters
    ----------
    capacity:
        Maximum number of chunks to retain.
    lod_distances:
        Distance thresholds at which to increase LOD level.
    """

    def __init__(self, capacity: int = 128, lod_distances: Iterable[float] = (64.0, 128.0, 256.0)) -> None:
        self.capacity = capacity
        self.lod_distances = list(lod_distances)
        self._cache: "OrderedDict[Tuple[int, int, int, int], Chunk]" = OrderedDict()

    # ------------------------------------------------------------------
    def _compute_lod(self, player_pos: Tuple[float, float, float], chunk_pos: Tuple[int, int, int]) -> int:
        cx = chunk_pos[0] * CHUNK_SIZE + CHUNK_SIZE / 2 - player_pos[0]
        cy = chunk_pos[1] * CHUNK_SIZE + CHUNK_SIZE / 2 - player_pos[1]
        cz = chunk_pos[2] * CHUNK_SIZE + CHUNK_SIZE / 2 - player_pos[2]
        dist = sqrt(cx * cx + cy * cy + cz * cz)
        lod = 0
        for i, threshold in enumerate(self.lod_distances, start=1):
            if dist >= threshold:
                lod = i
            else:
                break
        return lod

    # ------------------------------------------------------------------
    def get_chunk(
        self,
        chunk_pos: Tuple[int, int, int],
        player_pos: Tuple[float, float, float],
        loader: Callable[[Tuple[int, int, int], int], np.ndarray],
    ) -> Chunk:
        """Retrieve a chunk, loading and meshing it if necessary."""

        lod = self._compute_lod(player_pos, chunk_pos)
        key = (*chunk_pos, lod)

        if key in self._cache:
            self._cache.move_to_end(key)
            return self._cache[key]

        if len(self._cache) >= self.capacity:
            self._cache.popitem(last=False)

        voxels = loader(chunk_pos, lod)
        mesh = greedy_mesh(voxels)
        chunk = Chunk(position=chunk_pos, voxels=voxels, lod=lod, mesh=mesh)
        self._cache[key] = chunk
        return chunk


__all__ = ["Chunk", "ChunkCache", "CHUNK_SIZE"]

