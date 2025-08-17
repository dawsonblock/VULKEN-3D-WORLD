"""Lightweight physics helpers used in tests.

This package only provides a small set of utilities required by the unit
tests.  The C++ equivalents in ``src/physics_cpp`` are preferred for any
real gameplay logic.
"""

from __future__ import annotations

from typing import Any

import numpy as np

# Multiplier applied to ``max_speed`` when sprint input is active.
SPRINT_SPEED_MULTIPLIER: float = 1.6


def get_horizontal_speed(player: Any) -> float:
    """Return the magnitude of the player's horizontal velocity."""

    return float(np.linalg.norm(player.vel[[0, 2]]))


__all__ = ["SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]
