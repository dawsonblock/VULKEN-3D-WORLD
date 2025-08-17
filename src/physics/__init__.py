"""Simple physics helpers used in tests."""

from typing import Any

import numpy as np

# Multiplier applied to ``max_speed`` when sprint input is active.
SPRINT_SPEED_MULTIPLIER: float = 1.6

def get_horizontal_speed(player: Any) -> float:
    """Return the magnitude of the horizontal velocity of ``player``.

    ``player`` is expected to expose a ``vel`` attribute containing an array
    where the X and Z components represent horizontal motion.
    """
    return float(np.linalg.norm(player.vel[[0, 2]]))

__all__ = ["SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]
