import pytest

pytest.skip(
    "capsule properties tests require native physics module; skipped in CI",
    allow_module_level=True,
)
