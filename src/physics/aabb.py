"""Axis-aligned bounding box utilities used in tests."""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class AABB:
    """Minimal axis-aligned bounding box."""

    center: NDArray[np.float32]
    half: NDArray[np.float32]

    @property
    def min(self) -> NDArray[np.float32]:
        """Minimum corner of the box."""
        return self.center - self.half

    @property
    def max(self) -> NDArray[np.float32]:
        """Maximum corner of the box."""
        return self.center + self.half

    def moved(self, delta: NDArray[np.float32]) -> "AABB":
        """Return a new box translated by ``delta``."""
        return AABB(self.center + delta, self.half)

    def overlap_aabb(self, other: "AABB") -> bool:
        """Return ``True`` if this box overlaps ``other``."""
        return not (
            self.max[0] <= other.min[0]
            or self.min[0] >= other.max[0]
            or self.max[1] <= other.min[1]
            or self.min[1] >= other.max[1]
            or self.max[2] <= other.min[2]
            or self.min[2] >= other.max[2]
        )


__all__ = ["AABB"]

