import numpy as np

from src.physics.capsule import Capsule


def test_seg_properties() -> None:
    cap = Capsule(np.array([1.0, 2.0, 3.0], dtype=np.float32), 0.5, 1.0)
    np.testing.assert_array_equal(
        cap.seg_a, np.array([1.0, 2.5, 3.0], dtype=np.float32)
    )
    np.testing.assert_array_equal(
        cap.seg_b, np.array([1.0, 1.5, 3.0], dtype=np.float32)
    )

