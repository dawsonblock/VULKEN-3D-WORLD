"""Player controller capsule tests require native physics module."""

import pytest

pytest.skip(
    "player controller capsule tests require native physics module; skipped in CI",
    allow_module_level=True,
)

