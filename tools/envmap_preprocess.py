#!/usr/bin/env python3
"""Preprocess an environment map into IBL textures."""
import argparse
import numpy as np


def preprocess_envmap(src: str, dst: str) -> None:
    data = np.load(src)
    irradiance = data.mean(axis=(0, 1), keepdims=True)
    np.savez(dst, irradiance=irradiance, prefiltered=data)


def main() -> None:
    ap = argparse.ArgumentParser(description="Precompute IBL maps from an environment map (.npy)")
    ap.add_argument("src", help="Path to input environment map stored as a numpy array")
    ap.add_argument("dst", help="Output .npz file for generated textures")
    args = ap.parse_args()
    preprocess_envmap(args.src, args.dst)


if __name__ == "__main__":
    main()
