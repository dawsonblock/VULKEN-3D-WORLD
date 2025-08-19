"""Data structures for voxel chunks."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Tuple

import numpy as np

CHUNK_SIZE = 32


@dataclass
class Chunk:
    """Simple container for voxel and biome data of a chunk."""

    position: Tuple[int, int, int]
    voxels: np.ndarray  # shape (CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)
    biomes: np.ndarray  # shape (CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)


__all__ = ["Chunk", "CHUNK_SIZE"]
