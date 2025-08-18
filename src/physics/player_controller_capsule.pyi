from typing import Any

SPRINT_SPEED_MULTIPLIER: float


class PlayerControllerCapsule:
    max_speed: float
    vel: Any
    on_ground: bool
    pos: Any

    def __init__(self, *args: Any, **kwargs: Any) -> None: ...
    def update(self, *args: Any, **kwargs: Any) -> None: ...
    def set_input(self, *args: Any, **kwargs: Any) -> None: ...


def get_horizontal_speed(player: PlayerControllerCapsule) -> float: ...

