"""Minimal capsule geometry used in tests."""

from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by a center point."""

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """End point of the top hemisphere axis."""
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """End point of the bottom hemisphere axis."""
        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)
