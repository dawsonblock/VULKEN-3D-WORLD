import pytest

pytest.skip(
    "capsule ground collision tests require native extensions not built in CI",
    allow_module_level=True,
)
