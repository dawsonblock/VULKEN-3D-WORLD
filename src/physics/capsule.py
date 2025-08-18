


        main
        main
"""Minimal capsule representation for tests."""


import numpy as np
from dataclasses import dataclass

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

        main
from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray

import numpy as np
        main
from numpy.typing import NDArray
        main


@dataclass
class Capsule:

    """Vertical capsule defined by its center, half-height, and radius."""

  
  
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

        main
    """Simple vertical capsule defined by its center, half-height, and radius."""
        main
        main

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Center of the top spherical cap."""
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Center of the bottom spherical cap."""
        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)


__all__ = ["Capsule"]






        main
        main
        main
