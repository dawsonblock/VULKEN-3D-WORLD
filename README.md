# VoxelVK Elite ALL (Release-Ready Skeleton)

[![CI](https://github.com/<owner>/<repo>/actions/workflows/ci.yml/badge.svg)](https://github.com/<owner>/<repo>/actions/workflows/ci.yml)

This repo contains:
- Vulkan shader pipeline (SPIR-V under `build/spv/**`)
- AI palette + voxel mesher bridge (guarded if mesher headers absent)
- GPU blockizer compute shader (`shaders_vk/ai/blockize.comp.glsl`)
- Self-contained smoke test + CTest
- GitHub Actions CI (Linux & Windows) that installs Vulkan SDK, builds, tests, and publishes artifacts

## Local build
```bash
cmake -S . -B build -G Ninja -DENABLE_IMGUI_OVERLAY=ON -DVOXELVK_ENABLE_CUDA=OFF -DVOXELVK_ENABLE_TENSORRT=OFF
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The compute expects image formats:
- 3D occupancy: `VK_FORMAT_R8_UINT` (GENERAL)
- 2D height: `VK_FORMAT_R32_SFLOAT` (GENERAL)
- 2D biome: `VK_FORMAT_R8_UINT` (GENERAL)
- Output 3D blocks: `VK_FORMAT_R16_UINT` (GENERAL)


## Python setup

Some utilities and tests in `src/` and `tests/` require Python. Create a virtual environment and install the required packages:

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Run the test suite:

```bash
pytest

```

## Linting
To check JavaScript or TypeScript sources, run:
```bash
npx eslint .
```

## Samples

### demo_window
Build the sample:
```bash
cmake -S . -B build
cmake --build build --target demo_window
```
Run it:
```bash
./build/demo_window
```
