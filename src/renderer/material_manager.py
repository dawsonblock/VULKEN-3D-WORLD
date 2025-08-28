from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple, cast





# Number of scalar components used to represent a single material.
MATERIAL_COMPONENTS_COUNT = 5









        main
        main
        main
MATERIAL_COMPONENTS_COUNT = 5
        main
        main
        main

# Number of floats per material record when uploading to GPU buffers
MATERIAL_COMPONENTS_COUNT = 5  # 3 (albedo) + 1 (metallic) + 1 (roughness)

# Export constant to builtins for tests that reference it directly
import builtins as _builtins
_builtins.MATERIAL_COMPONENTS_COUNT = MATERIAL_COMPONENTS_COUNT

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

            albedo_list = props.get("albedo", [1.0, 1.0, 1.0])[:3]
            albedo = cast(Tuple[float, float, float], tuple(float(x) for x in albedo_list))






            albedo_raw = props.get("albedo", [1.0, 1.0, 1.0])
            albedo = cast(
                Tuple[float, float, float],
                (
                    float(albedo_raw[0]),
                    float(albedo_raw[1]),
                    float(albedo_raw[2]),
                ),
            )


            albedo_list = props.get("albedo", [1.0, 1.0, 1.0])[:3]
            albedo = cast(
                Tuple[float, float, float],
                tuple(float(x) for x in albedo_list),
            )

        main
        main
            albedo = cast(
                Tuple[float, float, float],
                tuple(float(x) for x in props.get("albedo", [1.0, 1.0, 1.0])),
            )






            raw_albedo = props.get("albedo", [1.0, 1.0, 1.0])[:3]
            albedo = cast(
                Tuple[float, float, float], tuple(float(x) for x in raw_albedo)
            )


            albedo_list = props.get("albedo", [1.0, 1.0, 1.0])
            albedo = (
                float(albedo_list[0]),
                float(albedo_list[1]),
                float(albedo_list[2]),
            )

            albedo_vals = [float(x) for x in props.get("albedo", [1.0, 1.0, 1.0])]
            albedo = (albedo_vals[0], albedo_vals[1], albedo_vals[2])
        main
        main
        main
        main
        main
        main
        main
        main
            metallic = float(props.get("metallic", 0.0))
            roughness = float(props.get("roughness", 1.0))
            mat = Material(idx, albedo, metallic, roughness)
            self._materials_by_name[name] = mat
            self._materials_by_id.append(mat)

    def get_material_id(self, name: str) -> int:


        """Return the numeric ID for a material name."""






        return self._materials_by_name[name].id

        main
        main
        main
        main
        if name not in self._materials_by_name:
            raise ValueError(f"Material {name} not found")
        return self._materials_by_name[name].id

    def get_material(self, material_id: int) -> Material:
        if not (0 <= material_id < len(self._materials_by_id)):
            raise IndexError(f"Material ID {material_id} out of range")
        return self._materials_by_id[material_id]

    def materials(self) -> List[Material]:
        return list(self._materials_by_id)

    def create_gpu_resources(self) -> List[Tuple[float, float, float, float, float]]:
        return [(*m.albedo, m.metallic, m.roughness) for m in self._materials_by_id]


__all__ = ["MaterialManager", "MATERIAL_COMPONENTS_COUNT"]
