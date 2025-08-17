

import numpy as np

        main
"""Definition of a vertical capsule shape used for collision queries."""

from __future__ import annotations

        main
from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
  center: np.ndarray

    """Vertical capsule represented by a center point, half-height and radius."""

    center: NDArray[np.float32]
        main
    half_height: float
    radius: float

    @property

    def seg_a(self) -> np.ndarray:
        """Center of the top spherical cap."""


    def seg_a(self) -> np.ndarray:
        main
        return self.center + np.array([0, self.half_height, 0], dtype=np.float32)

    @property
    def seg_b(self) -> np.ndarray: 
      
      """Center of the bottom spherical cap."""
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)

        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)


    def seg_a(self) -> np.ndarray:

    def seg_a(self) -> np.ndarray:

    def seg_a(self) -> NDArray[np.float32]:
        main
        main
        """Center of the top spherical cap."""

        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

    @property

    def seg_b(self) -> np.ndarray:
        """Center of the bottom spherical cap."""
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)


    def seg_b(self) -> np.ndarray:
        """Center of the bottom spherical cap."""
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)

    def seg_b(self) -> NDArray[np.float32]:
        """Center of the bottom spherical cap."""

        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)

        main
        main
        main
        main
