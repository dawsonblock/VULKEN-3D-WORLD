from __future__ import annotations

import numpy as np
import pytest

from src.ai.diff_voxelize import diff_voxelize


def _dummy_points() -> np.ndarray:
    return np.zeros((1, 3), dtype=np.float32)


def test_grid_shape_length() -> None:
    pts = _dummy_points()
    with pytest.raises(ValueError):
        diff_voxelize(pts, (4, 4))


def test_grid_shape_non_int() -> None:
    pts = _dummy_points()
    with pytest.raises(ValueError):
        diff_voxelize(pts, (4, 4, "a"))


def test_grid_shape_non_positive() -> None:
    pts = _dummy_points()
    with pytest.raises(ValueError):
        diff_voxelize(pts, (4, 0, 4))
