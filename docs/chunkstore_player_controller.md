# ChunkStore and Player Controller Capsule

This guide covers saving voxel chunks with `ChunkStore` and updating a capsule-based player controller.

## ChunkStore

`ChunkStore` manages compressed voxel data on disk. The implementation in [src/world/persistence.py](../src/world/persistence.py) supports `zstd`, `lz4`, or no compression. Choose a codec at construction time:

```python
from world.persistence import ChunkStore  # src/world/persistence.py

store = ChunkStore(root="world_save", codec="zstd", use_rle=True)
```

Saving chunks can be scheduled asynchronously and flushed later:

```python
store.save_async(chunk)
store.wait_all()  # ensure writes complete
```

Chunks are loaded on demand by coordinate:

```python
loaded = store.load_chunk(cx, cz)
if loaded:
    voxels = loaded["voxels"]
```

To use a different codec or disable run-length encoding:

```python
store = ChunkStore(root="world_save", codec="lz4", use_rle=False)
```

## PlayerControllerCapsule

`PlayerControllerCapsule` from [src/physics/player_controller_capsule.py](../src/physics/player_controller_capsule.py) updates position and velocity for a capsule-shaped player.

```python
from physics.player_controller_capsule import PlayerControllerCapsule  # src/physics/player_controller_capsule.py

controller = PlayerControllerCapsule(world_manager, spawn)
```

Inside the game loop, feed inputs and call `update`:

```python
while running:
    controller.set_input(read_inputs())
    controller.update(dt, forward, right)
    if controller.on_ground:
        ...
```

The controller applies acceleration, gravity, optional jumping, and step handling each frame.
