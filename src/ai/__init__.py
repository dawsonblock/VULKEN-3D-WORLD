"""AI helpers and differentiable voxelization bindings."""

from __future__ import annotations

import importlib
import sys
from types import ModuleType


def _load() -> ModuleType:
    """Load ``diff_voxelize`` ensuring a fresh import.

    The module may be compiled on the fly during tests, so a previous import
    could leave a stale entry in :data:`sys.modules`.  Clear both potential
    import paths before re-importing to guarantee the latest build is used.
    """

    for name in ("src.ai.diff_voxelize", "ai.diff_voxelize"):
        sys.modules.pop(name, None)

    module = importlib.import_module("src.ai.diff_voxelize")
    return importlib.reload(module)


diff_voxelize = _load().diff_voxelize

__all__ = ["diff_voxelize"]
