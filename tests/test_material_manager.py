from src.renderer.material_manager import MaterialManager


def test_material_manager_loads_and_ids_unique():
    mgr = MaterialManager()
    grass = mgr.get_material_id("GRASS")
    stone = mgr.get_material_id("STONE")
    assert grass != stone


def test_material_manager_gpu_resources_shape():
    mgr = MaterialManager()
    resources = mgr.create_gpu_resources()
    assert len(resources) == len(mgr.materials())
    assert all(len(r) == len(resources[0]) for r in resources)

