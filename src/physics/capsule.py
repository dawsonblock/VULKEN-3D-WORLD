"""Minimal capsule representation for tests."""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by its center, half-height and radius."""

    center: NDArray[np.float32]
    half_height: float
    radius: float
