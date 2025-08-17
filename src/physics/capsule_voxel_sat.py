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
    # Check multiple points around the bottom edge of the capsule
    num_samples = 8
    angle_step = 2 * np.pi / num_samples
    collision = False
    for i in range(num_samples):
        angle = i * angle_step
        x = cap.center[0] + cap.radius * np.cos(angle)
        z = cap.center[2] + cap.radius * np.sin(angle)
        if world.get_block_at_world_position(x, bottom, z):
            collision = True
            break
    # Also check the center point for completeness
    if not collision and world.get_block_at_world_position(cap.center[0], bottom, cap.center[2]):
        collision = True
    if collision:
        block_y = np.floor(bottom)
        offset = np.array([0.0, (block_y + 1) - bottom, 0.0], dtype=np.float32)
        cap.center = cap.center + offset
        return offset, True
    return np.zeros(3, dtype=np.float32), False
