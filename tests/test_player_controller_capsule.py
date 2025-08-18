"""Tests for the simplified :mod:`physics.player_controller_capsule` module."""

import numpy as np

from physics.player_controller_capsule import (
    PlayerControllerCapsule,
    SPRINT_SPEED_MULTIPLIER,
)


def test_get_horizontal_speed_clamps_then_reports_actual() -> None:
    """The first call to :meth:`get_horizontal_speed` clamps to max speed."""
    pc = PlayerControllerCapsule(object(), np.zeros(3, dtype=np.float32))
    forward = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    right = np.array([1.0, 0.0, 0.0], dtype=np.float32)

    # Engage sprint to exceed ``max_speed``.
    pc.set_input({"f": 1, "sprint": 1})
    for _ in range(60):
        pc.update(0.016, forward, right)

    clamped = pc.get_horizontal_speed()
    assert clamped <= pc.max_speed + 1e-3

    actual = pc.get_horizontal_speed()
    assert actual > pc.max_speed * SPRINT_SPEED_MULTIPLIER * 0.9

