"""Collision helpers for a capsule against a voxel world."""

from typing import Any, Optional, Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    """Minimal protocol for the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:  # pragma: no cover - simple protocol
        ...


def closest_point_on_aabb(p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]) -> NDArray[np.float32]:
    """Clamp point *p* to the axis-aligned box defined by *mn*/*mx*."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]) -> NDArray[np.float32]:
    """Return the closest point on the line segment *ab* to *p*."""
    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    t = np.clip(t, 0.0, 1.0)
    return a + t * ab


def capsule_box_penetration(
    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Compute penetration of ``cap`` against an axis-aligned box."""
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = np.linalg.norm(v)
    pen = cap.radius - dist
    if pen > 0.0:
        normal = (
            (v / (dist + 1e-9)).astype(np.float32)
            if dist > 1e-9
            else np.array([0, 1, 0], dtype=np.float32)
        )
        return True, normal, float(pen)
    return False, None, 0.0


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Move ``cap`` out of solid blocks and report ground contact."""
    total_off = np.zeros(3, dtype=np.float32)
    grounded = False
    for _ in range(max_iters):
        mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        bb_min = np.floor(mn).astype(int)
        bb_max = np.floor(mx).astype(int)
        max_pen = 0.0
        hit_n: Optional[NDArray[np.float32]] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    if not world.get_block_at_world_position(x, y, z):
                        continue
                    mn_v = np.array([x, y, z], dtype=np.float32)
                    mx_v = mn_v + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mn_v, mx_v)
                    if hit and pen > max_pen and n is not None:
                        max_pen = pen
                        hit_n = n
                        if n[1] > 0.9:
                            grounded = True
        if max_pen <= 0.0 or hit_n is None:
            break
        cap.center += hit_n * max_pen
        total_off += hit_n * max_pen
    return total_off, grounded
