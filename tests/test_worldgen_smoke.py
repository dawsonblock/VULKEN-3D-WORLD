from __future__ import annotations

import json
from pathlib import Path
from typing import Dict, List, Any


def load_json(path: Path) -> Dict[str, Any]:
    with path.open(encoding="utf-8") as f:
        return json.load(f)


def generate_world(structure: Dict[str, Any], palette: Dict[str, Any]) -> List[int]:
    blocks = structure.get("blocks", [])
    textures = palette.get("textures", {})
    missing_types = [block["type"] for block in blocks if block["type"] not in textures]
    if missing_types:
        raise ValueError(f"Missing block types in palette textures: {missing_types}")
    return [textures[block["type"]] for block in blocks]


def test_worldgen_smoke() -> None:
    repo_root = Path(__file__).resolve().parent.parent
    palette = load_json(repo_root / "assets/worldgen/palettes/basic.json")
    structure = load_json(repo_root / "assets/worldgen/templates/test_structure.json")
    world = generate_world(structure, palette)
    assert world == [0, 1]
