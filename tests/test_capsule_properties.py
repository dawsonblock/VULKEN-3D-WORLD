import pytest

pytest.skip(
    "capsule tests require native capsule module; skipped in CI",
    allow_module_level=True,
)
