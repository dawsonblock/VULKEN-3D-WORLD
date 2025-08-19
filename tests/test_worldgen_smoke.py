from __future__ import annotations

from pathlib import Path
import sys
import numpy as np

repo_root = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(repo_root / "src"))

from world.worldgen import WorldGenerator


def test_worldgen_deterministic() -> None:
    palette_path = repo_root / "assets/worldgen/palettes/basic.json"
    gen = WorldGenerator(palette_path)
    chunk1 = gen.generate_chunk(0, 0, size=4, height=8, seed=123)
    chunk2 = gen.generate_chunk(0, 0, size=4, height=8, seed=123)
    assert np.array_equal(chunk1["voxels"], chunk2["voxels"])
    chunk3 = gen.generate_chunk(0, 0, size=4, height=8, seed=42)
    assert not np.array_equal(chunk1["voxels"], chunk3["voxels"])
    assert np.all(chunk1["biomes"] == chunk1["biomes"][0, 0])

