# Quickstart Guide

This guide walks you through cloning the repository, running basic smoke tests, and executing a graphics sample.

## 1. Repository setup

Clone the repository and build it with CMake:

```bash
# Replace <your-repository-url> with the URL of your fork or the main repository on GitHub
git clone <your-repository-url>
cd VULKEN-3D-WORLD
cmake -S . -B build -G Ninja -DENABLE_IMGUI_OVERLAY=ON -DVOXELVK_ENABLE_CUDA=OFF -DVOXELVK_ENABLE_TENSORRT=OFF
cmake --build build -j
```

Set up the Python environment for utilities and tests:

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 2. Run smoke tests

After building, execute the bundled smoke tests:

```bash
ctest --test-dir build --output-on-failure
```

For Python-based checks, run:

```bash
pytest
```

## 3. Run graphics smoke sample

Build and run the headless graphics test:
```bash
cmake --build build --target smoke_graphics_headless
./build/smoke_graphics_headless
```

The run should complete without errors.
