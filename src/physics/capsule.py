import numpy as np
from dataclasses import dataclass


@dataclass
class Capsule:
    """Vertical capsule represented by a center point and radius.

    Parameters
    ----------
    center:
        Capsule midpoint ``(x, y, z)``.
    half_height:
        Distance from the center to the center of each spherical cap.
    radius:
        Radius of the capsule.
    """

    center: np.ndarray
    half_height: float
    radius: float

    @property
    def seg_a(self) -> np.ndarray:
        """Center of the top spherical cap."""
        return self.center + np.array([0, self.half_height, 0], dtype=np.float32)

    @property
    def seg_b(self) -> np.ndarray:
        """Center of the bottom spherical cap."""
        return self.center - np.array([0, self.half_height, 0], dtype=np.float32)

