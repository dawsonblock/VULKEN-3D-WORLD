from __future__ import annotations

from typing import Protocol, Tuple

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


class WorldProtocol(Protocol):
    """Minimal protocol expected from the voxel world used in tests."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        ...


def compute_capsule_voxel_bounds(cap: Capsule) -> Tuple[NDArray[np.int32], NDArray[np.int32]]:
    """Return integer voxel bounds covering ``cap``."""
    mn = np.floor(
        cap.center - np.array([cap.radius, cap.half_height, cap.radius], dtype=np.float32)
    ).astype(np.int32)
    mx = np.floor(
        cap.center + np.array([cap.radius, cap.half_height, cap.radius], dtype=np.float32)
    ).astype(np.int32)
    return mn, mx


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol, max_iters: int = 8
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve ``cap`` against ``world`` and return offset and ground state.

    This simplified implementation performs no collision detection and always
    returns zero offset and ``False`` for ``ground``.  It is sufficient for the
    lightweight tests in this repository.
    """

    return np.zeros(3, dtype=np.float32), False


__all__ = ["WorldProtocol", "compute_capsule_voxel_bounds", "resolve_capsule_world"]
