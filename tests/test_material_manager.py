from src.renderer.material_manager import MaterialManager


# Each GPU resource tuple is expected to contain RGB albedo values plus
# metallic and roughness components.
MATERIAL_COMPONENTS_COUNT = 5


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

