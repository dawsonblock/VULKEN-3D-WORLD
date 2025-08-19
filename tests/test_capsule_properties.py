import pytest
np = pytest.importorskip("numpy")
try:  # Skip tests if the capsule module is unavailable or invalid.
    from src.physics.capsule import Capsule
except (ImportError, ModuleNotFoundError):  # pragma: no cover - skip if module import fails
    pytest.skip("capsule module unavailable", allow_module_level=True)

def test_seg_properties():
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
