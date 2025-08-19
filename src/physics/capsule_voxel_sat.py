from __future__ import annotations

"""Collision helpers for a capsule against a voxel world."""

from typing import Protocol, Tuple
import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .voxel_solid import is_solid


class WorldProtocol(Protocol):
    """Minimal protocol expected from the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int: ...


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve vertical collisions for ``cap`` against ``world``.

    The function returns a tuple ``(offset, on_ground)`` where ``offset`` is the
    translation applied to ``cap`` to resolve penetration and ``on_ground``
    indicates whether the capsule is resting on solid ground.
    """
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    block = world.get_block_at_world_position(cap.center[0], bottom - 1e-4, cap.center[2])
    if is_solid(block):
        delta = -bottom
        off = np.array([0.0, delta, 0.0], dtype=np.float32)
        cap.center = cap.center + off
        return off, True
    return np.zeros(3, dtype=np.float32), False


__all__ = ["WorldProtocol", "resolve_capsule_world"]
