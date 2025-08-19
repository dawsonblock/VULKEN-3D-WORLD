from __future__ import annotations

import json
import math
from pathlib import Path
from typing import Tuple, List


def load_config(root: Path) -> dict:
    with (root / "assets/worldgen/noise_config.json").open(encoding="utf-8") as f:
        return json.load(f)


def generate_chunk(cfg: dict, chunk_x: int, chunk_z: int, size: int = 4) -> Tuple[List[int], List[int]]:
    freq = cfg["frequency"]
    amp = cfg["amplitude"]
    plains = cfg["plains_block"]
    mountain = cfg["mountain_block"]
    biome: List[int] = []
    vox: List[int] = []
    for z in range(size):
        for x in range(size):
            nx = (chunk_x * size + x) * freq
            nz = (chunk_z * size + z) * freq
            n = math.sin(nx) * math.cos(nz) * amp
            b = 0 if n > 0 else 1
            biome.append(b)
            vox.append(plains if b == 0 else mountain)
    return biome, vox


def test_noise_generation() -> None:
    root = Path(__file__).resolve().parent.parent
    cfg = load_config(root)
    biome, vox = generate_chunk(cfg, 0, 0)
    assert len(biome) == 16
    assert len(vox) == 16
    assert all(v in {cfg["plains_block"], cfg["mountain_block"]} for v in vox)
