"""Differentiable voxelization utilities with Python bindings."""

from __future__ import annotations

import ctypes
import subprocess
import sys
import shutil
from pathlib import Path
from typing import Tuple

import numpy as np

ROOT = Path(__file__).resolve().parent
LIB_NAMES = {
    "linux": "diff_voxelize.so",
    "linux2": "diff_voxelize.so",
    "darwin": "diff_voxelize.dylib",
    "win32": "diff_voxelize.dll",
}


def _load_lib() -> ctypes.CDLL:
    system = sys.platform
    lib_name = LIB_NAMES.get(system)
    if lib_name is None:
        raise RuntimeError(f"Unsupported platform: {system}")
    lib_path = ROOT / lib_name
    if not lib_path.exists():
        src = ROOT / "diff_voxelize.cpp"
        gpp = shutil.which("g++")
        if gpp is None:
            raise RuntimeError(
                "g++ compiler not found. Install g++ or ensure a pre-built diff_voxelize library is available."
            )
        cmd = [gpp, "-std=c++17", "-shared"]
        if system != "win32":
            cmd.append("-fPIC")
        cmd += [str(src), "-o", str(lib_path)]
        try:
            subprocess.check_call(cmd)
        except subprocess.CalledProcessError as exc:
            raise RuntimeError("Failed to compile diff_voxelize.cpp") from exc
    try:
        return ctypes.CDLL(str(lib_path))
    except OSError as exc:
        raise RuntimeError(f"Failed to load library: {lib_path}") from exc


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
