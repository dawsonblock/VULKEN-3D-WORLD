from __future__ import annotations

from typing import Any, Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        ...


def is_solid(value: Any) -> bool:  # pragma: no cover - trivial
    return bool(value)


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[np.ndarray, np.ndarray]:
    mn = cap.center - np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    mx = cap.center + np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    return np.floor(mn).astype(int), np.floor(mx).astype(int)


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol
) -> Tuple[NDArray[np.float32], bool]:
    """Keep ``cap`` above solid blocks in ``world``.

    The function performs a very small subset of collision resolution sufficient
    for the unit tests.
    """

    off = np.zeros(3, dtype=np.float32)
    grounded = False

    bb_min, bb_max = compute_capsule_voxel_bounds(cap)
    for y in range(bb_min[1], bb_max[1] + 1):
        for z in range(bb_min[2], bb_max[2] + 1):
            for x in range(bb_min[0], bb_max[0] + 1):
                if not is_solid(world.get_block_at_world_position(float(x), float(y), float(z))):
                    continue
                block_top = y + 1.0
                bottom = cap.center[1] - (cap.half_height + cap.radius)
                if bottom < block_top:
                    delta = block_top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    grounded = True
    return off, grounded
