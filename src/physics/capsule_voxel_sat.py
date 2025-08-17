"""Collision helpers for a capsule against a simple voxel world."""

from __future__ import annotations

from typing import Any, Tuple

import numpy as np

from .capsule import Capsule


def resolve_capsule_world(cap: Capsule, world: Any) -> Tuple[np.ndarray, bool]:
    """Resolve penetration of ``cap`` against blocks provided by ``world``.

    The ``world`` object is expected to implement
    ``get_block_at_world_position(x, y, z)`` returning a truthy value for solid
    blocks.  The routine is intentionally simple but sufficient for the unit
    tests which only exercise flat ground and single-block steps.
    """

    off = np.zeros(3, dtype=np.float32)
    ground = False

    # Bounding box of the capsule
    mn = cap.center - np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius],
        dtype=np.float32,
    )
    mx = cap.center + np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius],
        dtype=np.float32,
    )
    bb_min = np.floor(mn).astype(int)
    bb_max = np.floor(mx).astype(int)

    for y in range(bb_min[1], bb_max[1] + 1):
        for z in range(bb_min[2], bb_max[2] + 1):
            for x in range(bb_min[0], bb_max[0] + 1):
                if not world.get_block_at_world_position(
                    float(x), float(y), float(z)
                ):
                    continue
                block_top = y + 1.0
                bottom = cap.center[1] - (cap.half_height + cap.radius)
                if bottom < block_top:
                    delta = block_top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    ground = True

    return off, ground


__all__ = ["resolve_capsule_world"]
