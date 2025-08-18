"""Shared helpers for player movement physics.

These modules are retained for reference only. C++ equivalents live in
``src/physics_cpp`` and should be preferred for production use.
"""

from __future__ import annotations

from typing import Any

import numpy as np


# Multiplier applied to ``max_speed`` when sprint input is active.
SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(player: Any) -> float:
    """Return the horizontal speed of the player.

    Parameters
    ----------
    player:
        Object with a ``vel`` attribute representing velocity as a numpy array.
        Only the X and Z components are considered.

    Returns
    -------
    float
        Magnitude of the horizontal velocity vector.
    """

    return float(np.linalg.norm(player.vel[[0, 2]]))


__all__ = ["SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]

