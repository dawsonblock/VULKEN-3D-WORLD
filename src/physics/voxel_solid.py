class BlockType:
    """Enumeration of built-in block types used in tests."""

    AIR = 0


# Registry mapping block type IDs to property dictionaries.
BLOCK_PROPERTIES: dict[int, dict[str, bool]] = {}


def is_solid(block_type: int) -> bool:
    """Return ``True`` if ``block_type`` represents a solid block."""
    props = BLOCK_PROPERTIES.get(block_type)
    if props is None:
        return block_type != BlockType.AIR
    return bool(props.get("solid", block_type != BlockType.AIR))


__all__ = ["BlockType", "BLOCK_PROPERTIES", "is_solid"]
