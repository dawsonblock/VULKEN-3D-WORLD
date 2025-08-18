from __future__ import annotations

from typing import Dict


class BlockType:
    """Enumeration of built-in block types."""

    AIR = 0


# Simple registry mapping block type IDs to property dictionaries.
BLOCK_PROPERTIES: Dict[int, dict] = {}


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` represents a solid block."""
    props = BLOCK_PROPERTIES.get(block_type)
    if props is None:
        return block_type != BlockType.AIR
    return bool(props.get("solid", block_type != BlockType.AIR))


__all__ = ["BlockType", "BLOCK_PROPERTIES", "is_solid"]
