import numpy as np
from src.physics.capsule import Capsule


def test_capsule_segment_endpoints():
    cap = Capsule(center=np.array([1.0, 2.0, 3.0], dtype=np.float32), half_height=1.0, radius=0.5)
    np.testing.assert_array_equal(cap.seg_a, np.array([1.0, 3.0, 3.0], dtype=np.float32))
    np.testing.assert_array_equal(cap.seg_b, np.array([1.0, 1.0, 3.0], dtype=np.float32))
