
import numpy as np
def rle_encode(arr: np.ndarray):
    flat = arr.ravel()
    diffs = np.r_[True, flat[1:] != flat[:-1]]
    vals = flat[diffs]
    counts = np.diff(np.r_[np.nonzero(diffs)[0], flat.size])
    return vals.astype(np.uint8), counts.astype(np.int32), arr.shape
def rle_decode(vals, counts, shape):
    flat = np.repeat(vals, counts).astype(np.uint8)
    return flat.reshape(shape)
