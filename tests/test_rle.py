import numpy as np
from src.world.rle import rle_encode, rle_decode

def test_rle_roundtrip_small_arrays():
    arrays = [
        np.array([0, 0, 0, 1, 1, 2], dtype=np.uint8),
        np.array([[1, 1], [2, 2]], dtype=np.uint8),
        np.arange(8, dtype=np.uint8).reshape((2, 2, 2)),
    ]
    for arr in arrays:
        vals, counts, shape = rle_encode(arr)
        out = rle_decode(vals, counts, shape)
        assert np.array_equal(out, arr)
