"""Management of loaded voxel chunks with LRU eviction."""
from __future__ import annotations

from collections import OrderedDict
from typing import Iterable, Tuple

import numpy as np

from .chunk import Chunk, CHUNK_SIZE


class ChunkManager:
    """Keep track of loaded chunks around a player position.

    Chunks are generated on demand using the provided ``generator``. The
    manager maintains an :class:`~collections.OrderedDict` to track usage
    order and evicts the least recently used chunks when they fall outside
    the active radius or exceed ``max_chunks``.
    """

    def __init__(self, generator: ChunkGeneratorProtocol, radius: int = 1, max_chunks: int = 64):
        self.generator = generator
        self.radius = radius
        self.max_chunks = max_chunks
        self.chunks: "OrderedDict[Tuple[int, int, int], Chunk]" = OrderedDict()

    # ------------------------------------------------------------------
    def _chunk_coords(self, pos: Iterable[float]) -> Tuple[int, int, int]:
        arr = np.floor(np.asarray(pos, dtype=float) / CHUNK_SIZE).astype(int)
        return int(arr[0]), int(arr[1]), int(arr[2])

    def get_chunk(self, cx: int, cy: int, cz: int) -> Chunk:
        key = (cx, cy, cz)
        if key not in self.chunks:
            self.chunks[key] = self.generator.generate(cx, cy, cz)
        self.chunks.move_to_end(key)
        return self.chunks[key]

    def update(self, player_pos: Iterable[float]) -> None:
        """Ensure chunks around ``player_pos`` are loaded and others evicted."""

        center = self._chunk_coords(player_pos)
        needed = set()
        for dx in range(-self.radius, self.radius + 1):
            for dy in range(-self.radius, self.radius + 1):
                for dz in range(-self.radius, self.radius + 1):
                    key = (center[0] + dx, center[1] + dy, center[2] + dz)
                    needed.add(key)
                    self.get_chunk(*key)
        # Remove chunks that are no longer needed
        for key in list(self.chunks.keys()):
            if key not in needed:
        for key in set(self.chunks.keys()) - needed:
            del self.chunks[key]
        # Enforce capacity via LRU eviction
        while len(self.chunks) > self.max_chunks:
            self.chunks.popitem(last=False)


__all__ = ["ChunkManager"]
