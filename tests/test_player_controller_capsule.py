import numpy as np
import pytest

try:
    from src.physics.player_controller_capsule import (
        PlayerControllerCapsule,
        SPRINT_SPEED_MULTIPLIER,
        get_horizontal_speed,
    )
except Exception:
    pytest.skip(
        "player controller capsule requires native physics extensions; skipped in CI",
        allow_module_level=True,
    )


def test_horizontal_speed_zero_initially() -> None:
    controller = PlayerControllerCapsule(
        world_manager=None,
        spawn=np.zeros(3, dtype=np.float32),
    )
    assert get_horizontal_speed(controller) == 0.0
    assert SPRINT_SPEED_MULTIPLIER > 1.0
