"""Differentiable voxelization utilities with Python bindings."""

from __future__ import annotations

import ctypes
import subprocess
from pathlib import Path
from typing import Tuple

import numpy as np

ROOT = Path(__file__).resolve().parent
LIB_PATH = ROOT / "diff_voxelize.so"


def _load_lib() -> ctypes.CDLL:
    if not LIB_PATH.exists():
        src = ROOT / "diff_voxelize.cpp"
        subprocess.check_call(
            [
                "g++",
                "-std=c++17",
                "-shared",
                "-fPIC",
                str(src),
                "-o",
                str(LIB_PATH),
            ]
        )
    return ctypes.CDLL(str(LIB_PATH))


_lib: ctypes.CDLL | None = None


def diff_voxelize(points: np.ndarray, grid_shape: Tuple[int, int, int]) -> tuple[np.ndarray, np.ndarray]:
    """Voxelize ``points`` into a soft occupancy grid.

    Parameters
    ----------
    points:
        Array of shape ``(N, 3)`` representing 3D point positions.
    grid_shape:
        Dimensions of the output volume ``(width, height, depth)``.

    Returns
    -------
    occupancy:
        ``np.ndarray`` of shape ``grid_shape`` with accumulated soft occupancy.
    gradients:
        ``np.ndarray`` of shape ``(N, 3)`` containing gradients of the summed occupancy
        w.r.t. each input point position.
    """

    points = np.asarray(points)
    if points.ndim != 2 or points.shape[1] != 3:
        raise ValueError(f"`points` must have shape (N, 3), got {points.shape}")
    pts = np.ascontiguousarray(points, dtype=np.float32)
    n, _ = pts.shape

    try:
        w, h, d = grid_shape
    except Exception as exc:  # pragma: no cover - defensive
        raise ValueError("`grid_shape` must be a sequence of three integers") from exc
    if any(not isinstance(dim, int) for dim in (w, h, d)):
        raise ValueError("`grid_shape` must be a sequence of three integers")
    if any(dim <= 0 for dim in (w, h, d)):
        raise ValueError("`grid_shape` dimensions must be positive")

    global _lib
    if _lib is None:  # pragma: no cover - library compiled once
        _lib = _load_lib()
        _lib.diff_voxelize.argtypes = [
            ctypes.POINTER(ctypes.c_float),  # points
            ctypes.POINTER(ctypes.c_float),  # occupancy out
            ctypes.POINTER(ctypes.c_float),  # point gradients out
            ctypes.c_int,  # n_points
            ctypes.c_int,  # width
            ctypes.c_int,  # height
            ctypes.c_int,  # depth
        ]
        _lib.diff_voxelize.restype = None

    occ = np.zeros((w, h, d), dtype=np.float32)
    grad = np.zeros((n, 3), dtype=np.float32)
    _lib.diff_voxelize(
        pts.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        occ.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        grad.ctypes.data_as(ctypes.POINTER(ctypes.c_float)),
        n,
        w,
        h,
        d,
    )
    return occ, grad


__all__ = ["diff_voxelize"]
