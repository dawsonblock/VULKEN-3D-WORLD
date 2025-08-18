"""Utilities to determine whether voxel blocks are solid."""
"""Utilities to determine whether voxel blocks are solid."""

from __future__ import annotations

from typing import Dict


class BlockType:
    """Enumeration of basic block types."""

    AIR = 0


# Map of block type to property dictionary.
BLOCK_PROPERTIES: Dict[int, dict] = {}


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` should be considered solid."""
    props = BLOCK_PROPERTIES.get(block_type)
    if props is None:
        return block_type != BlockType.AIR
    return bool(props.get("solid", block_type != BlockType.AIR))


__all__ = ["BlockType", "BLOCK_PROPERTIES", "is_solid"]
