
import numpy as np
from typing import Any, Optional, Tuple

from .capsule import Capsule


def closest_point_on_aabb(
    p: np.ndarray, mn: np.ndarray, mx: np.ndarray
) -> np.ndarray:
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: np.ndarray, a: np.ndarray, b: np.ndarray
) -> np.ndarray:
    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: np.ndarray, mx: np.ndarray
) -> Tuple[bool, Optional[np.ndarray], float]:
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = float(np.linalg.norm(v))
    pen = float(cap.radius - dist)
    if pen > 0.0:
        n: np.ndarray = (
            v / (dist + 1e-9)
            if dist > 1e-8
            else np.array([0, 1, 0], dtype=np.float32)
        )
        return True, n, pen
    return False, None, 0.0


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[np.ndarray, bool]:
    mn = cap.center - np.array(
        [
            cap.radius,
            cap.half_height + cap.radius,
            cap.radius,
        ],
        dtype=np.float32,
    )
    mx = cap.center + np.array(
        [
            cap.radius,
            cap.half_height + cap.radius,
            cap.radius,
        ],
        dtype=np.float32,
    )
    bb_min = np.floor(mn).astype(int)
    bb_max = np.floor(mx).astype(int)

    total_offset = np.zeros(3, dtype=np.float32)
    ground = False
    for _ in range(max_iters):
        max_pen = 0.0
        hit_n: Optional[np.ndarray] = None
        contact_n: Optional[np.ndarray] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    bt = world.get_block_at_world_position(
                        float(x), float(y), float(z)
                    )
                    if bt == 0:
                        continue
                    mnv = np.array([x, y, z], dtype=np.float32)
                    mxv = mnv + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mnv, mxv)
                    if hit and pen > max_pen:
                        max_pen, hit_n = pen, n
                        contact_n = n
        if max_pen <= 1e-6 or hit_n is None:
            break
        off = hit_n * max_pen
        cap.center += off
        total_offset += off
        if contact_n is not None and contact_n[1] > 0.7:
            ground = True
    return total_offset, ground
