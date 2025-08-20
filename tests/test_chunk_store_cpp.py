import pytest

pytest.skip(
    "chunk store roundtrip requires building C++ components; skipped in CI",
    allow_module_level=True,
)
