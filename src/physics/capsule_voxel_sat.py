"""Collision helpers for resolving a capsule against a voxel world."""

from typing import Any, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


def resolve_capsule_world(
    cap: Capsule, world: Any
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve capsule collision against a voxel world.

    The ``world`` object is expected to provide a
    ``get_block_at_world_position(x, y, z)`` method returning a truthy value
    when the queried position is inside solid ground.  The function adjusts the
    capsule vertically so that it rests on top of the highest solid block it
    overlaps and reports whether the capsule is grounded.
    """

    bottom = cap.center[1] - cap.half_height - cap.radius
    off = np.zeros(3, dtype=np.float32)
    grounded = False

    sample_y = bottom - 1e-4
    if world.get_block_at_world_position(
        cap.center[0], sample_y, cap.center[2]
    ):
        top = np.floor(sample_y) + 1.0
        if top > bottom:
            off[1] = top - bottom
            cap.center[1] += off[1]
        grounded = True
        off[1] = 0.0

    return off, grounded
