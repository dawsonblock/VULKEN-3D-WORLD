import importlib
import pytest


def test_import_raises_import_error() -> None:
    with pytest.raises(ImportError):
        importlib.import_module("src.physics.player_controller_capsule")
