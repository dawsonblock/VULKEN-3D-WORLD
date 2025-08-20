from __future__ import annotations

from .material_manager import MaterialManager






def build_brdf_lut(material_name: str, manager: MaterialManager, *args, **kwargs):


def build_brdf_lut(material_name: str, manager: MaterialManager = _manager, *args, **kwargs) -> None:
    """Build a BRDF lookup texture for the given material."""

def build_brdf_lut(material_name: str, manager: MaterialManager, *args, **kwargs):



        main
_manager = MaterialManager()


def build_brdf_lut(
    material_name: str,
    manager: MaterialManager = _manager,
    *args,
    **kwargs,
) -> None:
    """Build a BRDF lookup texture for the given material.


    This stub validates that the material exists and raises
    ``NotImplementedError`` as a placeholder for integration with a real
    renderer.
    """

        main
        main
        main
        main
    """Build a BRDF lookup texture for the given material."""
        main

        main
    material_id = manager.get_material_id(material_name)
    raise NotImplementedError(
        f"Implement using your rendering backend (material id {material_id})."
    )


__all__ = ["build_brdf_lut"]
