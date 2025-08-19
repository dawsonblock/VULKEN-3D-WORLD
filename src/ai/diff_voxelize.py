"""Differentiable voxelization utilities with Python bindings.

The module lazily builds a small C++ extension at import time using ``g++``
with C++17 support. Two environment variables modify this behavior:

``DIFF_VOXELIZE_LIB``
    Absolute path to a prebuilt ``diff_voxelize`` shared library. When set,
    compilation is skipped and the given library is loaded directly.

``DIFF_VOXELIZE_BUILD_DIR``
    Directory where the shared library should be built. Defaults to the module
    directory. A ``diff_voxelize.so`` file will be created inside it if it does
    not already exist.

If neither environment variable is provided and compilation fails (for
example because ``g++`` is missing), importing this module will raise a
``RuntimeError`` explaining the failure.
"""

from __future__ import annotations

import ctypes
import subprocess
import os
from pathlib import Path
from typing import Tuple

import numpy as np

ROOT = Path(__file__).resolve().parent
_ENV_LIB = os.environ.get("DIFF_VOXELIZE_LIB")
_ENV_BUILD_DIR = os.environ.get("DIFF_VOXELIZE_BUILD_DIR")
_BUILD_DIR = Path(_ENV_BUILD_DIR) if _ENV_BUILD_DIR else ROOT
LIB_PATH = Path(_ENV_LIB) if _ENV_LIB else _BUILD_DIR / "diff_voxelize.so"


def _load_lib() -> ctypes.CDLL:
    """Load the ``diff_voxelize`` shared library, compiling if necessary."""
    if _ENV_LIB:
        if not LIB_PATH.exists():
            raise FileNotFoundError(
                f"DIFF_VOXELIZE_LIB set to {LIB_PATH} but file does not exist"
            )
        return ctypes.CDLL(str(LIB_PATH))

    if not LIB_PATH.exists():
        src = ROOT / "diff_voxelize.cpp"
        _BUILD_DIR.mkdir(parents=True, exist_ok=True)
        try:
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
        except (OSError, subprocess.CalledProcessError) as exc:
            raise RuntimeError(
                "Failed to compile diff_voxelize library. Ensure g++ is installed "
                "or provide a prebuilt library via DIFF_VOXELIZE_LIB."
            ) from exc
    return ctypes.CDLL(str(LIB_PATH))


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
    w, h, d = grid_shape
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
