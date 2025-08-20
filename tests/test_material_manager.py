import pytest

try:
    from src.renderer.material_manager import MaterialManager, MATERIAL_COMPONENTS_COUNT
except Exception:
    pytest.skip(
        "material manager requires renderer extensions; skipped in CI",
        allow_module_level=True,
    )


def test_material_manager_loads_and_ids_unique() -> None:
    mgr = MaterialManager()
    grass = mgr.get_material_id("GRASS")
    stone = mgr.get_material_id("STONE")
    assert grass != stone


def test_material_manager_gpu_resources_shape() -> None:
    mgr = MaterialManager()
    resources = mgr.create_gpu_resources()
    assert len(resources) == len(mgr.materials())
    assert all(len(r) == MATERIAL_COMPONENTS_COUNT for r in resources)
    expected_len = len(resources[0]) if resources else 0
    assert all(len(r) == expected_len for r in resources)
