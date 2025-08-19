import numpy as np

from src.render.culling import frustum_cull
from src.render.indirect_batch import MultiDrawIndirectBatch
from src.world.greedy_mesher import greedy_mesh


def test_frustum_culling_and_batch() -> None:
    vp = np.identity(4)
    chunks = [
        (1, (0.0, 0.0, 0.0), (1.0, 1.0, 1.0)),
        (2, (10.0, 10.0, 10.0), (11.0, 11.0, 11.0)),
    ]
    visible = frustum_cull(chunks, vp)
    assert visible == [1]

    mesh = greedy_mesh(np.ones((1, 1, 1), dtype=np.uint8))
    batch = MultiDrawIndirectBatch()
    batch.add_mesh(mesh)
    commands = batch.build()
    assert len(commands) == 1
    assert commands[0].count == len(mesh.indices)

