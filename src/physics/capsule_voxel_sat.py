"""Collision helpers for a capsule against a voxel world using simple SAT tests."""

from __future__ import annotations

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule


def resolve_capsule_world(
    cap: Capsule, world: object
) -> tuple[NDArray[np.float32], bool]:
    """Placeholder collision resolution.

    Returns a zero offset and ``False`` for ground contact.
    """

    return np.zeros(3, dtype=np.float32), False


__all__ = ["resolve_capsule_world"]

