from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple, cast

MATERIAL_COMPONENTS_COUNT = 5


@dataclass
class Material:
    """Simple container for material properties."""

    id: int
    albedo: Tuple[float, float, float]
    metallic: float
    roughness: float


class MaterialManager:
    """Loads material definitions and exposes GPU friendly data."""

    def __init__(self, config_path: Path | str | None = None) -> None:
        root = Path(__file__).resolve().parents[2]
        default = root / "assets" / "config" / "materials.json"
        self._config_path = Path(config_path) if config_path else default
        self._materials_by_name: Dict[str, Material] = {}
        self._materials_by_id: List[Material] = []
        self._load()

    def _load(self) -> None:
        data = json.loads(self._config_path.read_text())
        mats = data.get("materials", {})
        self._materials_by_name.clear()
        self._materials_by_id.clear()
        for idx, (name, props) in enumerate(mats.items()):
            albedo = cast(
                Tuple[float, float, float],
                tuple(float(x) for x in props.get("albedo", [1.0, 1.0, 1.0])),
            )
            metallic = float(props.get("metallic", 0.0))
            roughness = float(props.get("roughness", 1.0))
            mat = Material(idx, albedo, metallic, roughness)
            self._materials_by_name[name] = mat
            self._materials_by_id.append(mat)

    def get_material_id(self, name: str) -> int:
        """Return the numeric ID for a material name."""

        if name not in self._materials_by_name:
            raise ValueError(f"Material {name} not found")
        return self._materials_by_name[name].id

    def get_material(self, material_id: int) -> Material:
        """Fetch material properties by ID."""

        if not (0 <= material_id < len(self._materials_by_id)):
            raise IndexError(
                f"Material ID {material_id} is out of bounds. "
                f"Valid IDs are between 0 and {len(self._materials_by_id) - 1}."
            )
        return self._materials_by_id[material_id]

    def materials(self) -> List[Material]:
        """Return all loaded materials."""

        return list(self._materials_by_id)

    def create_gpu_resources(self) -> List[Tuple[float, float, float, float, float]]:
        """Package materials into a flat list suitable for GPU upload."""

        return [
            (*m.albedo, m.metallic, m.roughness) for m in self._materials_by_id
        ]


__all__ = ["Material", "MaterialManager", "MATERIAL_COMPONENTS_COUNT"]
