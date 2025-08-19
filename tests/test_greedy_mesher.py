import numpy as np

from src.world.greedy_mesher import greedy_mesh


def test_greedy_meshing_merges_faces() -> None:
    vox = np.ones((2, 2, 1), dtype=np.uint8)
    mesh = greedy_mesh(vox)
    assert mesh.vertices.shape[0] == 24
    assert mesh.indices.shape[0] == 36

