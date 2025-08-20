"""Minimal capsule representation for tests."""

from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by center, half-height, and radius."""

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def top(self) -> NDArray[np.float32]:
        return self.center + np.array([0, self.half_height, 0], dtype=np.float32)

    @property
    def bottom(self) -> NDArray[np.float32]:
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)

    def moved(self, delta: NDArray[np.float32]) -> Capsule:
        return Capsule(self.center + delta, self.half_height, self.radius)


__all__ = ["Capsule"]

