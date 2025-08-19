# Building `diff_voxelize`

The differentiable voxelizer exposes a small native library used by
`src/ai/diff_voxelize.py`. No binaries are checked into the repository;
build the library for your platform using the script described below.
Prebuilt artifacts may be provided in project releases for convenience.

## Prerequisites

- A C++17 compatible compiler (`g++` or `clang++`)
- For Windows builds, the `mingw-w64` toolchain

## Building from source

```bash
python scripts/build_diff_voxelize.py
```

The script writes the platform appropriate library next to the source
file. If compilation fails, the script surfaces the compiler output so it
can be inspected.
