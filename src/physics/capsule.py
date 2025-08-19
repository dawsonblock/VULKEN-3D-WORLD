from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by its center, half height and radius.

    The capsule is aligned along the Y axis.  ``center`` represents the middle
    of the cylindrical part, ``half_height`` is the half length of this
    cylinder and ``radius`` is the radius of the spherical caps and the
    cylinder.
    """

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Center of the top spherical cap."""
        return self.center + np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Center of the bottom spherical cap."""
        return self.center - np.array(
            [0.0, self.half_height, 0.0], dtype=np.float32
        )


__all__ = ["Capsule"]
