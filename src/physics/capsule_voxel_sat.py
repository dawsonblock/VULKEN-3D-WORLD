"""Collision helpers for a capsule against a voxel world using SAT tests."""

from __future__ import annotations

from typing import Optional, Protocol, Tuple, cast

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    """Minimal protocol required from the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int: ...


def closest_point_on_aabb(
    p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on the axis-aligned box defined by ``mn`` and ``mx``."""

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
    """Check penetration of ``cap`` against an axis-aligned box."""

    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = np.linalg.norm(v)
    pen = cap.radius - dist
    if pen > 0.0:
        normal = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, cast(NDArray[np.float32], normal), float(pen)
    return False, None, 0.0


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[np.ndarray, np.ndarray]:
    """Return integer min/max voxel coordinates overlapped by ``cap``."""

    mn = cap.center - np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    mx = cap.center + np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    return np.floor(mn).astype(int), np.floor(mx).astype(int)


def resolve_capsule_world(cap: Capsule, world: WorldProtocol) -> tuple[NDArray[np.float32], bool]:
    """Keep ``cap`` above solid blocks in ``world`` and report displacement and ground state."""

    off = np.zeros(3, dtype=np.float32)
    ground = False
    mn, mx = compute_capsule_voxel_bounds(cap)
    for y in range(mn[1], mx[1] + 1):
        for z in range(mn[2], mx[2] + 1):
            for x in range(mn[0], mx[0] + 1):
                if not is_solid(world.get_block_at_world_position(float(x), float(y), float(z))):
                    continue
                block_top = y + 1.0
                bottom = cap.center[1] - (cap.half_height + cap.radius)
                if bottom < block_top:
                    delta = block_top - bottom
                    cap.center[1] += delta
                    off[1] += delta
                    ground = True
    return off, ground


__all__ = [
    "WorldProtocol",
    "closest_point_on_aabb",
    "closest_point_on_segment",
    "capsule_box_penetration",
    "compute_capsule_voxel_bounds",
    "resolve_capsule_world",
]

