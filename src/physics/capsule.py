"""Lightweight capsule geometry helpers.

This module exposes a :class:`Capsule` dataclass used by the tests.  The
capsule represents a vertical capsule aligned with the Y axis and is
characterised by its centre, half-height and radius.
"""

from __future__ import annotations

from dataclasses import dataclass
import numpy as np
from numpy.typing import NDArray


@dataclass
class Capsule:
    """Simple vertical capsule defined by its centre, half-height and radius.

    Parameters
    ----------
    center:
        The midpoint of the capsule in world space.
    half_height:
        Half the height of the cylindrical part of the capsule.
    radius:
        Radius of both the cylindrical body and its spherical caps.
    """

    center: NDArray[np.float32]
    half_height: float
    radius: float

    def __post_init__(self) -> None:
        """Ensure the centre array uses ``float32`` precision."""
        self.center = self.center.astype(np.float32)

    @property
    def seg_a(self) -> NDArray[np.float32]:
        """Return the centre of the top spherical cap."""
        return self.center + np.array([0.0, self.half_height, 0.0], dtype=np.float32)

    @property
    def seg_b(self) -> NDArray[np.float32]:
        """Return the centre of the bottom spherical cap."""
        return self.center - np.array([0.0, self.half_height, 0.0], dtype=np.float32)


__all__ = ["Capsule"]
