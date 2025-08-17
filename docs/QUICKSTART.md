# Quickstart

## Prerequisites
- [Vulkan SDK](https://vulkan.lunarg.com/) 1.3 or newer
- [CMake](https://cmake.org/) 3.24+

## Build
```bash
cmake -S . -B build
cmake --build build -j
```

## Run samples
Run the smoke test:
```bash
./build/smoke_headless
```
Launch the viewer:
```bash
./build/demo_window
```
