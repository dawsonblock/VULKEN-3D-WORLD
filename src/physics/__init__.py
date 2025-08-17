"""Shared helpers for player movement physics.

These modules are retained for reference only. C++ equivalents live in
``src/physics_cpp`` and should be preferred for production use.
"""

from typing import Any

import numpy as np

SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(player: Any) -> float:
    """Return the horizontal speed magnitude of *player*."""
    return float(np.linalg.norm(player.vel[[0, 2]]))


__all__ = ["SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]
