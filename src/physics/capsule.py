from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by a center point, half-height, and radius."""

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Top point of the capsule's inner segment."""
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Bottom point of the capsule's inner segment."""
        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)


# Historically both ``Capsule`` and ``CapsulePy`` were defined.  The latter is
# kept as an alias for backward compatibility with older imports.
CapsulePy = Capsule

__all__ = ["Capsule", "CapsulePy"]
