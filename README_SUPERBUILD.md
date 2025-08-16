# SUPERBUILD_VULKEN_CSM

## Prerequisites
- CMake 3.24+
- Ninja
- Optional SDKs: [Vulkan SDK](https://vulkan.lunarg.com/), CUDA, TensorRT
- Python 3 for helper scripts

## Usage
```bash
# merge external snapshots
python scripts/perform_merge.py external/VULKEN-3D-WORLD some-dest

# one-command build
./scripts/build.sh
```

### Configuration toggles
- `VOXELVK_ENABLE_CUDA` – enable CUDA acceleration
- `VOXELVK_ENABLE_TENSORRT` – enable TensorRT features

### Known issues
- Vulkan targets are skipped when the SDK is missing
- CUDA/TensorRT require matching drivers

### Troubleshooting
- Ensure `external/VULKEN-3D-WORLD` is checked out:
  `gh repo clone VULKEN-3D/VULKEN-3D-WORLD external/VULKEN-3D-WORLD`
- Run `cmake --build build --target superbuild_check` for diagnostics
