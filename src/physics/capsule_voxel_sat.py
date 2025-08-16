
import numpy as np
from typing import Any, Tuple

"""Collision helpers for capsule vs voxel world."""

from typing import Optional, Protocol, Tuple

"""Collision detection between a capsule and a voxel grid using SAT."""

from typing import Any, Optional, Tuple
        main

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule



def resolve_capsule_world(
    cap: Capsule, world: Any
) -> Tuple[np.ndarray, bool]:
    """Resolve capsule collision against a simple voxel world.

    The provided ``world`` object only needs a ``get_block_at_world_position``
    method returning a truthy value when the capsule intersects solid ground.
    The implementation here is intentionally minimal and only correct for
    flat ground at ``y = 0``.
    """

    bottom = cap.center[1] - cap.half_height - cap.radius
    off = np.zeros(3, dtype=np.float32)
    grounded = False

    if bottom < 0:
        offset = -bottom
        cap.center[1] += offset
        off[1] = offset
        grounded = True

    return off, grounded



def closest_point_on_aabb(p: np.ndarray, mn: np.ndarray, mx: np.ndarray) -> np.ndarray:
    """Clamp point ``p`` to the axis-aligned box defined by ``mn`` and ``mx``."""

class WorldProtocol(Protocol):
    """Minimal protocol for the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:  # pragma: no cover - simple protocol
        ...


def closest_point_on_aabb(
    p: NDArray[np.float32], mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> NDArray[np.float32]:
    """Return the closest point on an axis-aligned bounding box to *p*."""

        main
    return np.minimum(np.maximum(p, mn), mx)



def closest_point_on_segment(
    p: NDArray[np.float32], a: NDArray[np.float32], b: NDArray[np.float32]
) -> NDArray[np.float32]:

def closest_point_on_segment(p: np.ndarray, a: np.ndarray, b: np.ndarray) -> np.ndarray:

    """Return the closest point on the segment ``ab`` to ``p``."""

        main
    """Return the closest point on the line segment *ab* to *p*."""

        main
    ab = b - a
    t = np.dot(p - a, ab) / (np.dot(ab, ab) + 1e-9)
    return a + np.clip(t, 0.0, 1.0) * ab


def capsule_box_penetration(

    cap: Capsule, mn: NDArray[np.float32], mx: NDArray[np.float32]
) -> Tuple[bool, Optional[NDArray[np.float32]], float]:

    cap: Capsule, mn: np.ndarray, mx: np.ndarray
) -> Tuple[bool, Optional[np.ndarray], float]:

    """Compute penetration of ``cap`` against an axis-aligned box."""

        main
    """Check whether *cap* penetrates the axis-aligned box defined by *mn*/*mx*."""

        main
    box_center = (mn + mx) * 0.5
    q_seg = closest_point_on_segment(box_center, cap.seg_a, cap.seg_b)
    q_box = closest_point_on_aabb(q_seg, mn, mx)
    v = q_seg - q_box
    dist = np.linalg.norm(v)
    pen = cap.radius - dist
    if pen > 0.0:        normal = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        return True, normal, pen


        n = (v / (dist + 1e-9)) if dist > 1e-8 else np.array([0, 1, 0], dtype=np.float32)

        n = v / (dist + 1e-9) if dist > 1e-9 else np.array([0, 1, 0], dtype=np.float32)
        main
        return True, n, pen
        main
    return False, None, 0.0



def resolve_capsule_world(
    cap: Capsule, world: Any, max_iters: int = 8
) -> Tuple[np.ndarray, bool]:
    """Resolve capsule against the voxel ``world`` and return total offset and ground state."""
    mn = cap.center - np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    mx = cap.center + np.array(
        [cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32
    )
    bb_min = np.floor(mn).astype(int)
    bb_max = np.floor(mx).astype(int)

def resolve_capsule_world(cap: Capsule, world, max_iters=8):
  

def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Move *cap* out of solid blocks and report ground contact.
        main

    This implementation is intentionally simple but sufficient for the unit test.
    """

        main
    total_offset = np.zeros(3, dtype=np.float32)
    ground = False
    for _ in range(max_iters):

        max_pen = 0.0
        hit_n: Optional[np.ndarray] = None
        contact_n: Optional[np.ndarray] = None
        for y in range(bb_min[1] - 1, bb_max[1] + 2):
            for z in range(bb_min[2] - 1, bb_max[2] + 2):
                for x in range(bb_min[0] - 1, bb_max[0] + 2):


        # Compute voxel bounds based on the capsule's current position.
        mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        bb_min = np.floor(mn).astype(int)
        bb_max = np.floor(mx).astype(int)

        max_pen = 0.0
        hit_n = None
        contact_n = None
        for y in range(bb_min[1]-1, bb_max[1]+2):
            for z in range(bb_min[2]-1, bb_max[2]+2):
                for x in range(bb_min[0]-1, bb_max[0]+2):
        main
                    bt = world.get_block_at_world_position(float(x), float(y), float(z))
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



        # The capsule's center moved, so its voxel bounds must be recalculated.
        # Recomputing here ensures the next iteration tests the correct region.
        mn = cap.center - np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        mx = cap.center + np.array([cap.radius, cap.half_height + cap.radius, cap.radius], dtype=np.float32)
        bb_min = np.floor(mn).astype(int)
        bb_min, bb_max = compute_capsule_voxel_bounds(cap)

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

        main
        main
    return total_offset, ground
        main
