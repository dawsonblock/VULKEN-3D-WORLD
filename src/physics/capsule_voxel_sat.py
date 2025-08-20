from __future__ import annotations

from typing import Optional, Protocol, Tuple

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
    p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Clamp point ``p`` to the box defined by ``mn`` and ``mx``."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on the segment ``ab`` to ``p``."""
    ab = b - a
    t = float(np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9))
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(
    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Check penetration of ``cap`` against an axis-aligned box."""
    center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    delta = q_seg - q_box
    dist = float(np.linalg.norm(delta))
    if dist < cap.radius:
        if dist > 1e-6:
            normal = delta / dist
        else:
            normal = np.array([0.0, 1.0, 0.0], dtype=np.float32)
        depth = cap.radius - dist
        return True, normal, depth
    return False, None, 0.0


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve collisions between ``cap`` and the voxel ``world``."""
    offset = np.zeros(3, dtype=np.float32)
    grounded = False
    for _ in range(max_iters):
        mn = cap.center - np.array(
            [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
        )
        mx = cap.center + np.array(
            [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
        )
        x0, y0, z0 = np.floor(mn).astype(int)
        x1, y1, z1 = np.floor(mx).astype(int)
        hit_any = False
        for x in range(x0, x1 + 1):
            for y in range(y0, y1 + 1):
                for z in range(z0, z1 + 1):
                    block = world.get_block_at_world_position(
                        x + 0.5, y + 0.5, z + 0.5
                    )
                    if not is_solid(block):
                        continue
                    box_mn = np.array([x, y, z], dtype=np.float32)
                    box_mx = box_mn + 1.0
                    hit, normal, depth = capsule_box_penetration(cap, box_mn, box_mx)
                    if hit and normal is not None:
                        move = normal * depth
                        cap.center = cap.center + move
                        offset += move
                        if normal[1] > 0.5:
                            grounded = True
                        hit_any = True
        if not hit_any:
            break
    return offset, grounded


__all__ = [
    "WorldProtocol",
    "closest_point_on_aabb",
    "closest_point_on_segment",
    "capsule_box_penetration",
    "resolve_capsule_world",
]
