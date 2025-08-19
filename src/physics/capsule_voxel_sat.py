"""Capsule to voxel collision helpers using simple SAT-style tests.

The routines here implement a very small subset of collision handling used
by the unit tests.  They operate on a :class:`~src.physics.capsule.Capsule`
against axis-aligned unit cubes queried from a ``WorldProtocol``.
"""

from __future__ import annotations

from typing import Optional, Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    """Protocol describing the voxel world queried by the collision routines."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        """Return the block identifier at the given world position."""
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32],
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Clamp point ``p`` to the axis-aligned box defined by ``mn`` and ``mx``."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: NDArray[np.float32],
    a: NDArray[np.float32],
    b: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Return the point on segment ``ab`` closest to ``p``."""
    ab = b - a
    denom = float(np.dot(ab, ab))
    if denom > 1e-6:
        t = float(np.dot(p - a, ab)) / denom
    else:  # segment is degenerate
        t = 0.0
    t = float(np.clip(t, 0.0, 1.0))
    return a + ab * t


def capsule_box_penetration(
    cap: Capsule,
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> Tuple[bool, NDArray[np.float32], float]:
    """Return collision information between ``cap`` and an AABB.

    The function returns ``(hit, normal, penetration)`` where ``hit`` indicates
    whether the capsule intersects the box, ``normal`` is the collision normal
    pointing out of the box and ``penetration`` is the penetration depth.
    """

    # Approximate the closest approach between the capsule segment and the box.
    seg_pt = closest_point_on_segment(closest_point_on_aabb(cap.center, mn, mx), cap.seg_a, cap.seg_b)
    box_pt = closest_point_on_aabb(seg_pt, mn, mx)
    delta = seg_pt - box_pt
    dist_sq = float(np.dot(delta, delta))
    if dist_sq >= cap.radius * cap.radius:
        return False, np.zeros(3, dtype=np.float32), 0.0
    dist = float(np.sqrt(dist_sq))
    if dist > 1e-6:
        normal = delta / dist
    else:
        normal = np.array([0.0, 1.0, 0.0], dtype=np.float32)
    penetration = cap.radius - dist
    return True, normal.astype(np.float32), float(penetration)


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[NDArray[np.int32], NDArray[np.int32]]:
    """Return the voxel-aligned bounds potentially intersected by ``cap``."""
    extents = np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    mn = np.floor(cap.center - extents).astype(np.int32)
    mx = np.floor(cap.center + extents).astype(np.int32)
    return mn, mx


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve ``cap`` against ``world``.

    Returns the offset applied to the capsule's centre and whether the capsule
    ended the resolution step on the ground.
    """

    total_offset = np.zeros(3, dtype=np.float32)
    ground = False
    for _ in range(max_iters):
        bb_min, bb_max = compute_capsule_voxel_bounds(cap)
        hit_n: Optional[NDArray[np.float32]] = None
        max_pen = 0.0
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    if not is_solid(world.get_block_at_world_position(float(x), float(y), float(z))):
                        continue
                    mnv = np.array([x, y, z], dtype=np.float32)
                    mxv = mnv + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mnv, mxv)
                    if hit and pen > max_pen:
                        max_pen, hit_n = pen, n
        if hit_n is None or max_pen <= 1e-6:
            break
        off = hit_n * max_pen
        cap.center += off
        total_offset += off
        if hit_n[1] > 0.7:
            ground = True
    return total_offset, ground


__all__ = [
    "WorldProtocol",
    "closest_point_on_aabb",
    "closest_point_on_segment",
    "capsule_box_penetration",
    "compute_capsule_voxel_bounds",
    "resolve_capsule_world",
]
