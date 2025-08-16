import numpy as np
from typing import Any, Tuple

from .capsule import Capsule


def resolve_capsule_world(
    cap: Capsule, world: Any
) -> Tuple[np.ndarray, bool]:
    """Resolve capsule collision against a simple voxel world.

    The provided ``world`` object only needs a ``get_block_at_world_position``
    method returning a truthy value when the capsule intersects solid ground.
    The implementation here is intentionally minimal and only correct for
    flat ground at ``y = 0``.
    """

    bottom = cap.center[1] - cap.half_height - cap.radius
    off = np.zeros(3, dtype=np.float32)
    grounded = False

    if bottom < 0:
        offset = -bottom
        cap.center[1] += offset
        off[1] = offset
        grounded = True

    return off, grounded

