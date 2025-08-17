"""Basic capsule geometry utilities.

This module defines a small :class:`Capsule` dataclass used throughout the
tests.  The previous version of this file contained many duplicated snippets
and stray text which made it impossible to import.  The implementation below
keeps things intentionally small and well typed so that other modules can rely
on it without additional helpers.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Vertical capsule represented by a center point, half-height and radius."""

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

