
"""Image based lighting helpers."""

from .material_manager import MaterialManager


def build_brdf_lut(material_name: str, manager: MaterialManager, *args, **kwargs):
    """Build a BRDF lookup texture for the given material."""

    material_id = manager.get_material_id(material_name)
    raise NotImplementedError(
        f"Implement using your rendering backend (material id {material_id})."
    )
