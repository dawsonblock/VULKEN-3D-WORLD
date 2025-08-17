import numpy as np

from src.world.rle import rle_encode, rle_decode


def _roundtrip(arr: np.ndarray) -> None:
    vals, counts, shape = rle_encode(arr)
    decoded = rle_decode(vals, counts, shape)
    assert decoded.shape == arr.shape
    assert np.array_equal(decoded, arr)


def test_small_arrays() -> None:
    _roundtrip(np.array([1, 2, 3], dtype=np.uint8))
    _roundtrip(np.array([[4, 5], [6, 7]], dtype=np.uint8))


def test_all_identical_values() -> None:
    _roundtrip(np.full((10,), 5, dtype=np.uint8))


def test_alternating_pattern() -> None:
    _roundtrip(np.array([0, 1] * 8, dtype=np.uint8))


def test_random_data() -> None:
    rng = np.random.default_rng(0)
    arr = rng.integers(0, 255, size=(7, 5), dtype=np.uint8)
    _roundtrip(arr)


def test_shape_preserved() -> None:
    arr = np.arange(24, dtype=np.uint8).reshape(2, 3, 4)
    vals, counts, shape = rle_encode(arr)
    decoded = rle_decode(vals, counts, shape)
    assert decoded.shape == (2, 3, 4)
    assert np.array_equal(decoded, arr)
