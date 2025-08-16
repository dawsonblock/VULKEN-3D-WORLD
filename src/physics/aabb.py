"""Axis-aligned bounding box helpers for collision tests."""

from dataclasses import dataclass
import numpy as np


@dataclass
class AABB:
    """Simple axis-aligned bounding box."""

    center: np.ndarray  # (x, y, z)
    half: np.ndarray    # (hx, hy, hz)

    @property
    def min(self) -> np.ndarray:
        """Minimum corner of the box."""
        return self.center - self.half

    @property
    def max(self) -> np.ndarray:
        """Maximum corner of the box."""
        return self.center + self.half

    def moved(self, delta: np.ndarray) -> "AABB":
        """Return a new box offset by ``delta``."""
        return AABB(self.center + delta, self.half)

    def overlap_aabb(self, other: "AABB") -> bool:
        """Check whether this box overlaps ``other``."""
        return not (
            self.max[0] <= other.min[0] or self.min[0] >= other.max[0]
            or self.max[1] <= other.min[1] or self.min[1] >= other.max[1]
            or self.max[2] <= other.min[2] or self.min[2] >= other.max[2]
        )
