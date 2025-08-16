import numpy as np
from typing import Any, Tuple

from .capsule import Capsule


def resolve_capsule_world(cap: Capsule, world: Any) -> Tuple[np.ndarray, bool]:
    """Very small helper used in tests to keep the capsule above solid blocks.

    The world object only needs ``get_block_at_world_position``. We check voxels
    overlapping the capsule's bounding box and push the capsule upward if a
    solid block is encountered. The routine is intentionally simplistic but
    sufficient for unit tests covering ground contact and basic step climbing.
    """

    off = np.zeros(3, dtype=np.float32)
    ground = False

    # Compute capsule bounding box
    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    bb_min = np.floor(mn).astype(int)
    bb_max = np.floor(mx).astype(int)

    for y in range(bb_min[1], bb_max[1] + 1):
        for z in range(bb_min[2], bb_max[2] + 1):
            for x in range(bb_min[0], bb_max[0] + 1):
                if not is_solid(world.get_block_at_world_position(float(x), float(y), float(z))):
                    continue
                # push upwards so bottom sits on top of block
                block_top = y + 1.0
                bottom = cap.center[1] - (cap.half_height + cap.radius)
                if bottom < block_top:
                    delta = block_top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    ground = True
    return off, ground
