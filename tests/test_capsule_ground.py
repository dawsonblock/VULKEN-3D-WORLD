import ctypes
import numpy as np
import pytest

from src.physics.capsule import Capsule, CapsulePy, resolve_capsule_world, WorldProtocol


class FlatWorld(WorldProtocol):
    """World with a solid plane at y = 0."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:  # noqa: D401 - simple stub
        return 1 if y < 0.0 else 0


def test_capsule_ground_collision() -> None:
    cap = Capsule(center=np.array([0.0, 0.2, 0.0], dtype=np.float32), half_height=0.5, radius=0.3)
    offset, grounded = resolve_capsule_world(cap, FlatWorld())

    assert grounded is True
    np.testing.assert_array_equal(offset, np.array([0.0, 0.6, 0.0], dtype=np.float32))
    assert np.allclose(cap.center, np.array([0.0, 0.8, 0.0], dtype=np.float32))

    cap_py = CapsulePy((ctypes.c_float * 3)(*cap.center), cap.half_height, cap.radius)
    assert list(cap_py.center) == pytest.approx(cap.center.tolist())
