import pytest

np = pytest.importorskip("numpy")

pytest.skip(
    "capsule property tests require additional setup; skipped in CI",
    allow_module_level=True,
)
