"""Capsule to voxel collision helpers used in tests."""

from __future__ import annotations

from typing import Protocol
import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32],
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Clamp point ``p`` to the box defined by ``mn`` and ``mx``."""
    return np.minimum(np.maximum(p, mn), mx)
