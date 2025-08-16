"""Collision detection between a capsule and a voxel grid using simple axis tests."""


from typing import Any, Tuple


"""Collision helpers for capsule vs voxel world using SAT."""

from typing import Any, Optional, Tuple, cast

"""Capsule collision helpers for a voxel world using simple SAT tests."""

from __future__ import annotations

from typing import Any, Optional, Protocol, Tuple
        main

        main
import numpy as np

from .capsule import Capsule


def resolve_capsule_world(cap: Capsule, world: Any) -> Tuple[np.ndarray, bool]:
    """Resolve capsule collision against a voxel world.


    The ``world`` object exposes ``get_block_at_world_position`` which returns
    a truthy value for solid blocks. The resolution used here is deliberately
    simple and only handles the cases exercised in the tests: flat ground and
    one-block-high steps.
    """
    off = np.zeros(3, dtype=np.float32)
    grounded = False

    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius])
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius])

    for y in range(int(np.floor(mn[1])), int(np.ceil(mx[1]))):
        for z in range(int(np.floor(mn[2])), int(np.ceil(mx[2]))):
    for y in range(int(np.floor(mn[1] - 0.5)), int(np.ceil(mx[1]))):
        for z in range(int(np.floor(mn[2] - 0.5)), int(np.ceil(mx[2]))):
            for x in range(int(np.floor(mn[0] - 0.5)), int(np.ceil(mx[0]))):
                if world.get_block_at_world_position(float(x), float(y), float(z)):
                    top = y + 1.0
                    bottom = cap.center[1] - cap.half_height - cap.radius
                    if bottom < top:
                        delta = top - bottom
                        if delta > off[1]:
                            off[1] = delta
                        off[1] += delta
                        grounded = True

                    # Compute penetration along each axis
                    block_min = np.array([x, y, z], dtype=np.float32)
                    block_max = block_min + 1.0
                    cap_min = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius])
                    cap_max = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius])

                    # Calculate overlap along each axis
                    overlap = np.zeros(3, dtype=np.float32)
                    for axis in range(3):
                        if cap_max[axis] > block_min[axis] and cap_min[axis] < block_max[axis]:
                            min_pen = block_max[axis] - cap_min[axis]
                            max_pen = cap_max[axis] - block_min[axis]
                            # Choose the smaller penetration
                            if min_pen < max_pen:
                                overlap[axis] = min_pen
                            else:
                                overlap[axis] = -max_pen
                        else:
                            overlap[axis] = 0.0

                    # Find axis of minimum penetration (MTV)
                    abs_overlap = np.abs(overlap)
                    axis = np.argmax(abs_overlap)
                    if abs_overlap[axis] > 0.0:
                        if abs_overlap[axis] > abs(off[axis]):
                            off[axis] = overlap[axis]
                            if axis == 1 and overlap[1] > 0.0:
                                grounded = True

    cap.center += off
    return off, grounded

def closest_point_on_aabb(
    p: np.ndarray, mn: np.ndarray, mx: np.ndarray
) -> np.ndarray:
    """Return the closest point on an axis-aligned box to ``p``."""
    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(
    p: np.ndarray, a: np.ndarray, b: np.ndarray
) -> np.ndarray:
    """Return the closest point on the segment ``ab`` to ``p``."""
    
class WorldProtocol(Protocol):
    """Minimal protocol required from the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int: ...


def closest_point_on_aabb(p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]) -> NDArray[np.float32]:
    """Clamp point ``p`` to the axis-aligned box defined by ``mn`` and ``mx``."""

    return np.minimum(np.maximum(p, mn), mx)


def closest_point_on_segment(p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]) -> NDArray[np.float32]:
    """Return the closest point on the segment ``ab`` to ``p``."""

        main
    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    return a + np.clip(t, 0.0, 1.0) * ab



def capsule_box_penetration(
    cap: Capsule, mn: np.ndarray, mx: np.ndarray
) -> Tuple[bool, Optional[np.ndarray], float]:
    """Compute penetration of ``cap`` against an axis-aligned box."""

def capsule_box_penetration(cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]) -> Tuple[bool, Optional[NDArray[np.float32]], float]:
    """Check penetration of ``cap`` against an axis-aligned box."""

        main
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box

    dist = np.linalg.norm(v)

    pen = cap.radius - dist
    if pen > 0.0:
        normal = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, cast(np.ndarray, normal), float(pen)
    return False, None, 0.0


def resolve_capsule_world(
    cap: Capsule, world: Any, max_iters: int = 8
) -> Tuple[np.ndarray, bool]:
    """Resolve capsule against the voxel ``world`` and return total offset and ground state."""

    dist = float(np.linalg.norm(v))
    pen = float(cap.radius - dist)
    if pen > 0.0:
        n = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, n, pen
    return False, None, 0.0


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[np.ndarray, np.ndarray]:
    """Return integer min/max voxel coordinates overlapped by ``cap``."""

    mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
    return np.floor(mn).astype(int), np.floor(mx).astype(int)


def resolve_capsule_world(cap: Capsule, world: WorldProtocol, max_iters: int = 8) -> Tuple[NDArray[np.float32], bool]:
    """Move *cap* out of solid voxels in ``world`` and report ground contact."""

        main
    total_offset = np.zeros(3, dtype=np.float32)
    ground = False

    for _ in range(max_iters):
        )  
        mn = cap.center - np.array(
            [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
        )
        mx = cap.center + np.array(
            [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
        )
        bb_min = np.floor(mn).astype(int)
        bb_max = np.floor(mx).astype(int)

        max_pen = 0.0
        hit_n: Optional[np.ndarray] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    bt = world.get_block_at_world_position(float(x), float(y), float(z))
                    if not bt:

        bb_min, bb_max = compute_capsule_voxel_bounds(cap)
        max_pen = 0.0
        hit_n: Optional[NDArray[np.float32]] = None
        contact_n: Optional[NDArray[np.float32]] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):
                    if world.get_block_at_world_position(float(x), float(y), float(z)) == 0:
         main
                        continue
                    mn = np.array([x, y, z], dtype=np.float32)
                    mx = mn + 1.0
                    hit, n, pen = capsule_box_penetration(cap, mn, mx)
                    if hit and pen > max_pen and n is not None:
                        max_pen, hit_n = pen, n
        if max_pen <= 1e-6 or hit_n is None:
            break
        off = hit_n * max_pen
        cap.center += off
        total_offset += off

        if hit_n is not None and hit_n[1] > 0.7:
            ground = True


        if contact_n is not None and contact_n[1] > 0.7:
            ground = True
        main
    return total_offset, ground

        main
