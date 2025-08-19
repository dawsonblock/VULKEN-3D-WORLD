"""AI helpers and differentiable voxelization bindings."""

from __future__ import annotations

import importlib
import sys
from types import ModuleType


def _load() -> ModuleType:
    """Import ``diff_voxelize`` with a clean cache.

    Any existing ``diff_voxelize`` modules are removed from ``sys.modules``
    before importing to ensure a fresh copy is loaded. ``importlib.reload``
    is used explicitly for clarity.
    """

    for name in ("src.ai.diff_voxelize", "ai.diff_voxelize"):
        sys.modules.pop(name, None)
    module = importlib.import_module("src.ai.diff_voxelize")
    return importlib.reload(module)


diff_voxelize = _load().diff_voxelize

__all__ = ["diff_voxelize"]

