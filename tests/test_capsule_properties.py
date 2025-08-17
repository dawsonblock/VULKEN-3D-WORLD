import numpy as np

from src.physics.capsule import Capsule


def test_seg_properties() -> None:
    cap = Capsule(
        center=np.array([1.0, 2.0, 3.0], dtype=np.float32),
        half_height=0.5,
        radius=1.0,
    )
    assert isinstance(type(cap).seg_a, property)
    assert isinstance(type(cap).seg_b, property)
    np.testing.assert_array_equal(
        cap.seg_a, np.array([1.0, 2.5, 3.0], dtype=np.float32)
    )
    np.testing.assert_array_equal(
        cap.seg_b, np.array([1.0, 1.5, 3.0], dtype=np.float32)
    )
