"""Collision helpers for resolving a capsule against a voxel world."""

from __future__ import annotations

from typing import Any, Optional, Tuple

import numpy as np

from .capsule import Capsule


def closest_point_on_aabb(p: np.ndarray, mn: np.ndarray, mx: np.ndarray) -> np.ndarray:
    """Clamp point ``p`` to the axis-aligned box defined by ``mn`` and ``mx``."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(p: np.ndarray, a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """Return the closest point on the line segment ``ab`` to ``p``."""
    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: np.ndarray, mx: np.ndarray
) -> Tuple[bool, Optional[np.ndarray], float]:
    """Compute penetration of ``cap`` against an axis-aligned box."""
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = np.linalg.norm(v)
    pen = cap.radius - dist
    if pen > 0.0:
        n = v / (dist + 1e-9) if dist > 1e-9 else np.array([0.0, 1.0, 0.0], dtype=np.float32)
        return True, n, float(pen)
    return False, None, 0.0


def resolve_capsule_world(cap: Capsule, world: Any, max_iters: int = 8) -> Tuple[np.ndarray, bool]:
    """Resolve ``cap`` against the voxel ``world`` and return total offset and ground state."""
    total_offset = np.zeros(3, dtype=np.float32)
    ground = False
    for _ in range(max_iters):
        mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        bb_min = np.floor(mn).astype(int)
        bb_max = np.floor(mx).astype(int)

        max_pen = 0.0
        hit_n: Optional[np.ndarray] = None
        for y in range(bb_min[1], bb_max[1] + 1):
            for z in range(bb_min[2], bb_max[2] + 1):
                for x in range(bb_min[0], bb_max[0] + 1):
                    bt = world.get_block_at_world_position(float(x), float(y), float(z))
                    if bt == 0:
                        continue
                    mnv = np.array([x, y, z], dtype=np.float32)
                    mxv = mnv + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mnv, mxv)
                    if hit and pen > max_pen:
                        max_pen = pen
                        hit_n = n
        if max_pen <= 1e-6 or hit_n is None:
            break
        off = hit_n * max_pen
        cap.center += off
        total_offset += off
        if hit_n[1] > 0.7:
            ground = True
    return total_offset, ground
