from __future__ import annotations

"""Collision helpers for a capsule against a voxel world using SAT tests."""

from typing import Optional, Protocol, Tuple, cast

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    """Minimal protocol expected from the world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        """Return the block type at the given world position."""
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32],
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Return the closest point on the AABB defined by ``mn`` and ``mx``."""
    return cast(NDArray[np.float32], np.minimum(np.maximum(p, mn), mx))


def closest_point_on_segment(
    p: NDArray[np.float32],
    a: NDArray[np.float32],
    b: NDArray[np.float32],
) -> NDArray[np.float32]:
    """Return the closest point on segment ``ab`` to point ``p``."""
    ab = b - a
    t = float(np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9))
    return cast(NDArray[np.float32], a + np.clip(t, 0.0, 1.0) * ab)


def capsule_box_penetration(
    cap: Capsule,
    mn: NDArray[np.float32],
    mx: NDArray[np.float32],
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Check penetration of ``cap`` against an axis-aligned box.

    Returns
    -------
    tuple
        ``(hit, normal, penetration_depth)``
    """
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = float(np.linalg.norm(v))
    pen = cap.radius - dist
    if pen > 0.0:
        normal = (
            v / (dist + 1e-9)
            if dist > 1e-9
            else np.array([0.0, 1.0, 0.0], dtype=np.float32)
        )
        return True, cast(NDArray[np.float32], normal), float(pen)
    return False, None, 0.0


def compute_capsule_voxel_bounds(
    cap: Capsule,
) -> Tuple[NDArray[np.int_], NDArray[np.int_]]:
    """Return integer min/max voxel coordinates overlapped by ``cap``."""
    mn = cap.center - np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius],
        dtype=np.float32,
    )
    mx = cap.center + np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius],
        dtype=np.float32,
    )
    return np.floor(mn).astype(int), np.floor(mx).astype(int)


def resolve_capsule_world(
    cap: Capsule,
    world: WorldProtocol,
    max_iters: int = 8,
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve ``cap`` against the voxel ``world``.

    Returns the total offset applied and whether the capsule ended on the
    ground.
    """
    total_offset = np.zeros(3, dtype=np.float32)
    ground = False
    for _ in range(max_iters):
        bb_min, bb_max = compute_capsule_voxel_bounds(cap)
        max_pen = 0.0
        hit_n: Optional[NDArray[np.float32]] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    bt = world.get_block_at_world_position(
                        float(x), float(y), float(z)
                    )
                    if not is_solid(bt):
                        continue
                    mnv = np.array([x, y, z], dtype=np.float32)
                    mxv = mnv + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mnv, mxv)
                    if hit and pen > max_pen:
                        max_pen, hit_n = pen, n
        if max_pen <= 1e-6 or hit_n is None:
            break
        off = hit_n * max_pen
        cap.center += off
        total_offset += off
        if hit_n is not None and hit_n[1] > 0.7:
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
