from __future__ import annotations

import json
import math
from pathlib import Path
from typing import Dict, Any

import numpy as np


class WorldGenerator:
    """Deterministic chunk generator using layered trigonometric noise."""

    def __init__(self, palette_path: Path | str) -> None:
        path = Path(palette_path)
        with path.open(encoding="utf-8") as f:
            palette: Dict[str, Any] = json.load(f)
        self.textures: Dict[str, int] = palette.get("textures", {})
        self.biomes: Dict[str, int] = palette.get("biomes", {})
        self._default_biome = self.biomes.get("plains", 0)

    @staticmethod
    def _noise(x: int, z: int, seed: int, freq: float) -> float:
        return 0.5 * (math.sin((x + seed) * freq) + math.cos((z - seed) * freq))

    def _height(self, x: int, z: int, seed: int) -> int:
        h = self._noise(x, z, seed, 0.1) * 10
        h += self._noise(x, z, seed + 1337, 0.2) * 5
        return int(abs(h)) % 16

    def generate_chunk(
        self, cx: int, cz: int, size: int = 16, height: int = 16, seed: int = 0
    ) -> Dict[str, np.ndarray]:
        voxels = np.zeros((size, height, size), dtype=np.uint8)
        biomes = np.full((size, size), self._default_biome, dtype=np.uint8)
        dirt = self.textures.get("dirt", 0)
        grass = self.textures.get("grass", 0)
        for dx in range(size):
            for dz in range(size):
                world_x = cx * size + dx
                world_z = cz * size + dz
                h = self._height(world_x, world_z, seed)
                top = min(h, height - 1)
                if top > 0:
                    voxels[dx, :top, dz] = dirt
                voxels[dx, top, dz] = grass
        return {"voxels": voxels, "biomes": biomes}

