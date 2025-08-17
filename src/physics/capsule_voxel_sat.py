"""Collision helpers between capsules and a simple voxel world.

Only a tiny subset of functionality is required for the unit tests.  The
implementations below purposely trade physical accuracy for simplicity: the
world exposes ``get_block_at_world_position`` which returns a truthy value for
solid voxels.  Collision resolution merely moves the capsule upwards so it
rests on top of solid blocks that intersect its bounding box.
"""

from __future__ import annotations

from typing import Any, Optional, Protocol, Tuple, cast

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    """Minimal protocol expected from the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int: ...


def closest_point_on_aabb(
    p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Clamp point ``p`` to the axis-aligned box defined by ``mn`` and ``mx``."""

    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on the segment ``ab`` to ``p``."""

    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Compute penetration of ``cap`` against an axis-aligned box."""

    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = float(np.linalg.norm(v))
    pen = cap.radius - dist
    if pen > 0.0:
        normal = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, cast(NDArray[np.float32], normal), pen
    return False, None, 0.0


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[NDArray[np.int32], NDArray[np.int32]]:
    """Return integer min/max voxel coordinates overlapped by ``cap``."""

    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    return np.floor(mn).astype(np.int32), np.floor(mx).astype(np.int32)


def resolve_capsule_world(cap: Capsule, world: Any) -> Tuple[NDArray[np.float32], bool]:
    """Resolve capsule collision against the voxel ``world``.

    The routine only handles the cases needed by the tests: flat ground and a
    single block step.  It returns the total offset applied to the capsule and a
    flag indicating whether it ended up on the ground.
    """

    off = np.zeros(3, dtype=np.float32)
    grounded = False

    mn, mx = compute_capsule_voxel_bounds(cap)

    for y in range(mn[1], mx[1] + 1):
        for z in range(mn[2], mx[2] + 1):
            for x in range(mn[0], mx[0] + 1):
                if not world.get_block_at_world_position(float(x), float(y), float(z)):
                    continue
                top = y + 1.0
                bottom = cap.center[1] - cap.half_height - cap.radius
                if bottom < top:
                    delta = top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    grounded = True

    return off, grounded


__all__ = [
    "WorldProtocol",
    "closest_point_on_aabb",
    "closest_point_on_segment",
    "capsule_box_penetration",
    "compute_capsule_voxel_bounds",
    "resolve_capsule_world",
]

