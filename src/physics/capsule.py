"""Simple capsule representation used in tests."""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule defined by its center, half-height and radius."""

    center: NDArray[np.float32]
    half_height: float
    radius: float

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Top point of the capsule's inner segment."""

        return np.array(
            [self.center[0], self.center[1] + self.half_height, self.center[2]],
            dtype=np.float32,
        )

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Bottom point of the capsule's inner segment."""

        return np.array(
            [self.center[0], self.center[1] - self.half_height, self.center[2]],
            dtype=np.float32,
        )


__all__ = ["Capsule"]

