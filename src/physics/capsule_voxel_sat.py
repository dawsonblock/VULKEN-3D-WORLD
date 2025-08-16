"""Collision detection between a capsule and a voxel grid using simple axis tests."""

from typing import Any, Tuple

import numpy as np

from .capsule import Capsule


def resolve_capsule_world(cap: Capsule, world: Any) -> Tuple[np.ndarray, bool]:
    """Resolve capsule collision against a voxel world.

    The ``world`` object exposes ``get_block_at_world_position`` which returns
    a truthy value for solid blocks. The resolution used here is deliberately
    simple and only handles the cases exercised in the tests: flat ground and
    one-block-high steps.
    """
    off = np.zeros(3, dtype=np.float32)
    grounded = False

    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius])
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius])

    for y in range(int(np.floor(mn[1])), int(np.ceil(mx[1]))):
        for z in range(int(np.floor(mn[2])), int(np.ceil(mx[2]))):
            for x in range(int(np.floor(mn[0])), int(np.ceil(mx[0]))):
                if world.get_block_at_world_position(float(x), float(y), float(z)):
                    top = y + 1.0
                    bottom = cap.center[1] - cap.half_height - cap.radius
                    if bottom < top:
                        delta = top - bottom
                        if delta > off[1]:
                            off[1] = delta
                            grounded = True

    cap.center[1] += off[1]
    return off, grounded
