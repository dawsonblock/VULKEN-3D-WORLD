from __future__ import annotations

import json
from pathlib import Path
from typing import Dict, List, Any


def load_json(path: Path) -> Dict[str, Any]:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def generate_world(structure: Dict[str, Any],
                   palette: Dict[str, Any],
                   materials: Dict[str, Any]) -> List[tuple[int, Dict[str, Any]]]:
    blocks = structure.get("blocks", [])
    textures = palette.get("textures", {})
    mat_lookup = {k.lower(): v for k, v in materials.items()}
    missing_textures = [b["type"] for b in blocks if b["type"] not in textures]
    if missing_textures:
        raise ValueError(f"Missing block types in palette textures: {missing_textures}")
    missing_materials = [b["type"] for b in blocks if b["type"] not in mat_lookup]
    if missing_materials:
        raise ValueError(f"Missing materials for block types: {missing_materials}")
    return [(textures[b["type"]], mat_lookup[b["type"]]) for b in blocks]


def test_worldgen_smoke() -> None:
    repo_root = Path(__file__).resolve().parent.parent
    palette = load_json(repo_root / "assets/worldgen/palettes/basic.json")
    structure = load_json(repo_root / "assets/worldgen/templates/test_structure.json")
    materials = load_json(repo_root / "assets/config/materials.json")["materials"]
    world = generate_world(structure, palette, materials)
    assert world[0][0] == 0 and world[1][0] == 1
    assert world[0][1]["albedo"] == [0.22, 0.35, 0.12]
