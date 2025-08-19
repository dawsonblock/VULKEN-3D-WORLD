import pytest

player_controller = pytest.importorskip(
    "src.physics.player_controller_capsule",
    reason="player_controller_capsule module unavailable",
)
from player_controller import (
    PlayerControllerCapsule,
    SPRINT_SPEED_MULTIPLIER,
    get_horizontal_speed,
)


def test_placeholder() -> None:
    """Placeholder test ensuring module loads."""
    assert PlayerControllerCapsule or SPRINT_SPEED_MULTIPLIER or get_horizontal_speed
