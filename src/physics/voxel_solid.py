"""Utilities to query whether a voxel block type is solid."""

from typing import Dict



class BlockType:



from typing import Dict


        main
# Adapt this to your engine's block registry

        main
class BlockType:
    """Enumeration of built-in block types."""

        main
    AIR = 0


BLOCK_PROPERTIES: Dict[int, dict] = {}


def is_solid(block_type: int) -> bool:
    props = BLOCK_PROPERTIES.get(block_type)
    if props is None:
        return block_type != BlockType.AIR
    return bool(props.get("solid", block_type != BlockType.AIR))
