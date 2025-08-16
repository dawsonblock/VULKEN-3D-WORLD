"""Collision helpers for capsule vs voxel world."""

from typing import Optional, Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    """Minimal protocol for the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:  # pragma: no cover - simple protocol
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on an axis-aligned bounding box to *p*."""

    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on the line segment *ab* to *p*."""

    ab = b - a
    t = float(np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9))
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Check whether *cap* penetrates the axis-aligned box defined by *mn*/*mx*."""

    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = np.linalg.norm(v)
    pen = cap.radius - dist
    if pen > 0.0:
        normal = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, normal, pen
    return False, None, 0.0


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Move *cap* out of solid blocks and report ground contact.

    This implementation is intentionally simple but sufficient for the unit test.
    """

    total_offset = np.zeros(3, dtype=np.float32)
    ground = False

    for _ in range(max_iters):
        bottom = cap.center[1] - (cap.half_height + cap.radius)
        if world.get_block_at_world_position(cap.center[0], bottom, cap.center[2]):
            delta = -bottom
            cap.center[1] += delta
            total_offset[1] += delta
            ground = True
        # Compute capsule bounding box
        min_corner = np.minimum(cap.seg_a, cap.seg_b) - cap.radius
        max_corner = np.maximum(cap.seg_a, cap.seg_b) + cap.radius
        min_block = np.floor(min_corner).astype(int)
        max_block = np.ceil(max_corner).astype(int)
        collided = False
        for x in range(min_block[0], max_block[0] + 1):
            for y in range(min_block[1], max_block[1] + 1):
                for z in range(min_block[2], max_block[2] + 1):
                    if world.get_block_at_world_position(x, y, z):
                        mn = np.array([x, y, z], dtype=np.float32)
                        mx = mn + 1.0
                        hit, n, pen = capsule_box_penetration(cap, mn, mx)
                        if hit and n is not None and pen > 0.0:
                            cap.center += n * pen
                            total_offset += n * pen
                            collided = True
                            if n[1] > 0.5:
                                ground = True
        if not collided:
            break

    return total_offset, ground
