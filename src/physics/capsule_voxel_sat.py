"""Collision helpers for a capsule against a voxel world."""

from __future__ import annotations

from typing import Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    """Minimal protocol expected from the world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int: ...


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[np.ndarray, np.ndarray]:
    """Return integer min/max voxel coordinates overlapped by ``cap``."""
    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    return np.floor(mn).astype(int), np.floor(mx).astype(int)


def resolve_capsule_world(cap: Capsule, world: WorldProtocol) -> tuple[np.ndarray, bool]:
    """Very small helper used in tests to keep the capsule above solid blocks."""
    off = np.zeros(3, dtype=np.float32)
    ground = False
    mn, mx = compute_capsule_voxel_bounds(cap)
    for y in range(mn[1], mx[1] + 1):
        for z in range(mn[2], mx[2] + 1):
            for x in range(mn[0], mx[0] + 1):
                if not is_solid(world.get_block_at_world_position(float(x), float(y), float(z))):
                    continue
                block_top = y + 1.0
                bottom = cap.center[1] - (cap.half_height + cap.radius)
                if bottom < block_top:
                    delta = block_top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    ground = True
    return off, ground


__all__ = ["WorldProtocol", "compute_capsule_voxel_bounds", "resolve_capsule_world"]
