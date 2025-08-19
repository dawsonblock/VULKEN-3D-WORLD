from __future__ import annotations
from dataclasses import dataclass
from typing import Any, Dict

@dataclass
class Texture:
    data: Any

class ResourceManager:
    def __init__(self) -> None:
        self.textures: Dict[str, Texture] = {}

    def add_texture(self, name: str, tex: Texture) -> None:
        self.textures[name] = tex

    def get_texture(self, name: str) -> Texture:
        return self.textures[name]

resources = ResourceManager()
