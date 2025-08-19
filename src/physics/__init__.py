from typing import Any
import numpy as np


SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(player: Any) -> float:
    return float(np.linalg.norm(player.vel[[0, 2]]))


__all__ = ["SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]
