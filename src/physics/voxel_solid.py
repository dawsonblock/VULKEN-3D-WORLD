

        main
"""Utilities to query whether a voxel block type is solid."""

from __future__ import annotations


"""Utilities to query whether a voxel block type is solid."""

from __future__ import annotations


"""Utilities to query whether a voxel block type is solid."""

from __future__ import annotations
         main

from __future__ import annotations

from typing import Dict
        main
        main

from typing import Dict



        main
        main
class BlockType:
    """Enumeration of built-in block types used in tests."""

    AIR = 0



# Registry describing block properties. Games can populate this as needed.


# Adapt this to your engine's block registry
BLOCK_PROPERTIES: Dict[int, Dict[str, bool]] = {}

# Simple registry mapping block type IDs to property dictionaries.
        main
BLOCK_PROPERTIES: Dict[int, dict] = {}
        main


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` represents a solid block."""

    props = BLOCK_PROPERTIES.get(block_type)
    if not props:
        return block_type != BlockType.AIR
    return bool(props.get("solid", block_type != BlockType.AIR))


__all__ = ["BlockType", "BLOCK_PROPERTIES", "is_solid"]






        main
        main
        main
        main
