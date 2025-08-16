"""Definition of a vertical capsule shape used for collision queries."""

import numpy as np
from dataclasses import dataclass


@dataclass
class Capsule:
    center: np.ndarray
    half_height: float
    radius: float

    @property
    def seg_a(self) -> np.ndarray:
        return self.center + np.array([0, self.half_height, 0], dtype=np.float32)

    @property
    def seg_b(self) -> np.ndarray:
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)
