"""Frustum culling utilities for chunk meshes."""

from __future__ import annotations

from typing import Iterable, List, Sequence, Tuple

import numpy as np


Plane = np.ndarray


def extract_planes(view_proj: Sequence[Sequence[float]]) -> List[Plane]:
    """Extract normalized frustum planes from a view-projection matrix."""

    m = np.array(view_proj, dtype=float).reshape(4, 4)
    planes = [
        m[3] + m[0],
        m[3] - m[0],
        m[3] + m[1],
        m[3] - m[1],
        m[3] + m[2],
        m[3] - m[2],
    ]
    return [p / np.sqrt(np.sum(p[:3]**2)) for p in planes]


def box_in_frustum(box_min: Sequence[float], box_max: Sequence[float], planes: Iterable[Plane]) -> bool:
    """Check if an axis-aligned bounding box intersects the frustum."""

    bmin = np.asarray(box_min, dtype=float)
    bmax = np.asarray(box_max, dtype=float)
    for p in planes:
        n = p[:3]
        d = p[3]
        # Positive vertex
        v = np.where(n >= 0, bmax, bmin)
        if np.dot(n, v) + d < 0:
            return False
    return True


def frustum_cull(
    chunks: Iterable[Tuple[int, Sequence[float], Sequence[float]]],
    view_proj: Sequence[Sequence[float]],
) -> List[int]:
    """Return IDs of chunks visible inside the frustum."""

    planes = extract_planes(view_proj)
    visible: List[int] = []
    for cid, bmin, bmax in chunks:
        if box_in_frustum(bmin, bmax, planes):
            visible.append(cid)
    return visible


__all__ = ["frustum_cull", "extract_planes", "box_in_frustum"]

