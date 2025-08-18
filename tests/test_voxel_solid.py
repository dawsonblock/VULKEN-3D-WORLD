from src.physics.voxel_solid import BlockType, BLOCK_PROPERTIES, is_solid


def test_is_solid_basic() -> None:
    assert not is_solid(BlockType.AIR)

    BLOCK_PROPERTIES[1] = {"solid": True}
    assert is_solid(1)

