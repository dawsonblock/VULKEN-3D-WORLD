"""Helpers for capsule vs. voxel collision tests."""

from __future__ import annotations

from typing import Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    """Minimal protocol expected from the world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32],
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Clamp point ``p`` to the box defined by ``mn`` and ``mx``."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: NDArray[np.float32],
    a: NDArray[np.float32],
    b: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Return the closest point on segment ``ab`` to ``p``."""
    ab = b - a
    t = float(np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9))
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[float, NDArray[np.float32]]:
    """Return dummy penetration depth and normal for capsule/box intersection."""
    closest = closest_point_on_aabb(cap.center, mn, mx)
    return 0.0, closest - cap.center
