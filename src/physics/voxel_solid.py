"""Utilities to query whether a voxel block type is solid."""

from typing import Dict


class BlockType:
    """Enumeration of built-in block types."""

    AIR = 0


BLOCK_PROPERTIES: Dict[int, dict] = {}


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` should be considered solid."""
    props = BLOCK_PROPERTIES.get(block_type)
    if props is not None:
        return bool(props.get("solid", block_type != BlockType.AIR))
    return block_type != BlockType.AIR

