# VULKEN/CSM Superbuild

This repository provides a meta-build that can assemble optional subprojects and run
basic diagnostics. Only the minimal IPC smoke test is guaranteed to build on all
machines; Vulkan, CUDA and TensorRT integrations are opt-in via CMake options.

## Prerequisites
* CMake 3.24+
* A C++20 compiler (MSVC, Clang or GCC)
* Ninja build system (recommended)
* Optional SDKs:
  * [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
  * CUDA Toolkit + TensorRT

## Usage
```bash
# fetch subprojects
python scripts/perform_merge.py --force

# configure and build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Toggle features
- `-DENABLE_VULKAN=ON|OFF`
- `-DVOXELVK_ENABLE_CUDA=ON|OFF`
- `-DVOXELVK_ENABLE_TENSORRT=ON|OFF` (implies CUDA)

## Troubleshooting
If the Vulkan SDK or CUDA toolkit are missing the configuration step will emit
warnings with links to the appropriate installers. Use the `superbuild_check`
target for a summary of detected toolchains and toggles:

```bash
cmake --build build --target superbuild_check
```
