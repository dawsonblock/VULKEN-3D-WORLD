from __future__ import annotations

from dataclasses import dataclass
import ctypes
from typing import Protocol, Tuple
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by its center, half-height and radius."""

    center: NDArray[np.float32]
    half_height: float
    radius: float


class CapsulePy(ctypes.Structure):
    """ctypes mirror of :class:`Capsule` used for C bindings."""

    _fields_ = [
        ("center", ctypes.c_float * 3),
        ("half_height", ctypes.c_float),
        ("radius", ctypes.c_float),
    ]


class WorldProtocol(Protocol):
    """Minimal protocol for a voxel world.

    Only the ``y`` coordinate is used in the simplified resolver below.
    ``get_block_at_world_position`` should return a non-zero value for
    solid blocks.
    """

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        ...


def resolve_capsule_world(
    cap: Capsule, world: WorldProtocol | None = None
) -> Tuple[NDArray[np.float32], bool]:
    """Resolve ``cap`` against a flat ground at ``y = 0``.

    Parameters
    ----------
    cap:
        Capsule to resolve. The ``center`` is modified in place when a
        correction occurs.
    world:
        Currently unused but kept for API compatibility.

    Returns
    -------
    offset, grounded:
        ``offset`` is the displacement applied to the capsule and
        ``grounded`` indicates whether it touched the ground.
    """

    bottom = cap.center[1] - (cap.half_height + cap.radius)
    if bottom < 0.0:
        delta = -bottom
        cap.center[1] += delta
        return np.array([0.0, delta, 0.0], dtype=np.float32), True
    return np.zeros(3, dtype=np.float32), False


__all__ = ["Capsule", "CapsulePy", "WorldProtocol", "resolve_capsule_world"]
