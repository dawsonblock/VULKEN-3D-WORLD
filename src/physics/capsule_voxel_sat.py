"""Simplified collision helpers for capsules in a voxel world."""

from __future__ import annotations

import numpy as np
from numpy.typing import NDArray

from typing import Any
from .capsule import Capsule


def resolve_capsule_world(cap: Capsule, world: Any) -> tuple[NDArray[np.float32], bool]:
    """Resolve intersection of ``cap`` with the ground defined by ``world``.

    The world is queried via ``get_block_at_world_position``.  If the bottom of
    the capsule is below the first solid block, the capsule is moved upward so
    that it rests on the surface.  The function returns the displacement vector
    applied to the capsule and a flag indicating whether the capsule was on the
    ground after resolution.
    """
    bottom = cap.center[1] - cap.half_height - cap.radius
    if world.get_block_at_world_position(cap.center[0], bottom, cap.center[2]):
        offset = np.array([0.0, -bottom, 0.0], dtype=np.float32)
        cap.center = cap.center + offset
        return offset, True
    return np.zeros(3, dtype=np.float32), False
