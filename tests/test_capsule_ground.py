import pytest

pytest.skip(
    "capsule ground tests require native physics module; skipped in CI",
    allow_module_level=True,
)
