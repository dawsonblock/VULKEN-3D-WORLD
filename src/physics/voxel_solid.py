"""Utilities to query whether a voxel block type is solid."""

from __future__ import annotations

from typing import Dict


class BlockType:
    """Enumeration of built-in block types used in tests."""

    AIR = 0


# Simple registry mapping block type IDs to property dictionaries.
BLOCK_PROPERTIES: Dict[int, Dict[str, bool]] = {}


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` represents a solid block."""
    try:
        props = BLOCK_PROPERTIES.get(block_type, {})
        return bool(props.get("solid", block_type != BlockType.AIR))
    except Exception:
        return block_type != BlockType.AIR


__all__ = ["BlockType", "BLOCK_PROPERTIES", "is_solid"]
