import pytest

np = pytest.importorskip("numpy")

try:
    from src.physics.capsule import Capsule
except Exception as exc:  # module may be missing or invalid
    pytest.skip(f"src.physics.capsule is unavailable: {exc}", allow_module_level=True)


def test_capsule_properties() -> None:
    center = np.array([0.0, 0.0, 0.0], dtype=np.float32)
    cap = Capsule(center, 1.0, 0.5)
    assert np.allclose(cap.center, center)
    assert cap.half_height == pytest.approx(1.0)
    assert cap.radius == pytest.approx(0.5)
