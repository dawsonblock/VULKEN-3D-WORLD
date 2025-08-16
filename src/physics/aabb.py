"""Axis-aligned bounding box helpers for collision tests."""

import numpy as np
from dataclasses import dataclass


@dataclass
class AABB:
    center: np.ndarray
    half: np.ndarray

    @property
    def min(self) -> np.ndarray:
        return self.center - self.half

    @property
    def max(self) -> np.ndarray:
        return self.center + self.half

    def moved(self, delta: np.ndarray) -> "AABB":
        return AABB(self.center + delta, self.half)

    def overlap_aabb(self, other: "AABB") -> bool:
        return not (
            self.max[0] <= other.min[0] or self.min[0] >= other.max[0] or
            self.max[1] <= other.min[1] or self.min[1] >= other.max[1] or
            self.max[2] <= other.min[2] or self.min[2] >= other.max[2]
        )
