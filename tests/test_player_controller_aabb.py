import numpy as np

from src.physics.player_controller import PlayerController, SPRINT_SPEED_MULTIPLIER


class FlatWorld:
    def get_block_at_world_position(self, x, y, z):
        return 1 if y < 0 else 0


class StepWorld:
    def get_block_at_world_position(self, x, y, z):
        if y < 0:
            return 1
        if 0 <= y < 1 and 1 <= x < 2:
            return 1
        return 0


def test_jump_and_land():
    world = FlatWorld()
    player = PlayerController(world, np.array([0.0, 1.2, 0.0], dtype=np.float32))
    forward = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    right = np.array([1.0, 0.0, 0.0], dtype=np.float32)

    for _ in range(10):
        player.update(0.1, forward, right)
        if player.on_ground:
            break
    assert player.on_ground

    player.set_input({"jump": 1})
    player.update(0.1, forward, right)
    assert player.vel[1] > 0
    assert not player.on_ground

    player.set_input({"jump": 0})
    for _ in range(50):
        player.update(0.1, forward, right)
        if player.on_ground:
            break
    assert player.on_ground
    assert abs(player.vel[1]) < 1e-6


def test_step_climb():
    world = StepWorld()
    player = PlayerController(world, np.array([0.0, 1.2, 0.0], dtype=np.float32), step_height=1.0)
    forward = np.array([1.0, 0.0, 0.0], dtype=np.float32)
    right = np.array([0.0, 0.0, 1.0], dtype=np.float32)

    for _ in range(10):
        player.update(0.1, forward, right)
        if player.on_ground:
            break
    player.set_input({"f": 1})
    player.update(0.1, forward, right)

    assert player.pos[1] > 1.2
    assert player.on_ground


def test_sprint_speed_limit():
    world = FlatWorld()
    player = PlayerController(world, np.array([0.0, 1.2, 0.0], dtype=np.float32))
    forward = np.array([1.0, 0.0, 0.0], dtype=np.float32)
    right = np.array([0.0, 0.0, 1.0], dtype=np.float32)

    for _ in range(10):
        player.update(0.1, forward, right)
        if player.on_ground:
            break
    player.set_input({"f": 1})
    for _ in range(60):
        player.update(0.1, forward, right)
    speed = float(np.linalg.norm(player.vel[[0, 2]]))
    assert speed <= player.max_speed + 1e-3

    player.set_input({"sprint": 1})
    for _ in range(60):
        player.update(0.1, forward, right)
    sprint_speed = float(np.linalg.norm(player.vel[[0, 2]]))
    assert sprint_speed <= player.max_speed * SPRINT_SPEED_MULTIPLIER + 1e-3
    assert sprint_speed > speed
