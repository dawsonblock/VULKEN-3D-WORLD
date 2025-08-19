"""Simple multi-draw-indirect batching helpers."""

from __future__ import annotations

from dataclasses import dataclass
from typing import List

from src.world.greedy_mesher import MeshBuffers


@dataclass
class DrawCommand:
    """CPU representation of a ``VkDrawIndexedIndirectCommand``."""

    count: int
    instance_count: int
    first_index: int
    vertex_offset: int
    first_instance: int


class MultiDrawIndirectBatch:
    """Accumulate draw commands for visible chunk meshes."""

    def __init__(self) -> None:
        self._commands: List[DrawCommand] = []
        self._index_offset = 0
        self._vertex_offset = 0

    def add_mesh(self, mesh: MeshBuffers) -> None:
        cmd = DrawCommand(
            count=int(len(mesh.indices)),
            instance_count=1,
            first_index=self._index_offset,
            vertex_offset=self._vertex_offset,
            first_instance=len(self._commands),
        )
        self._commands.append(cmd)
        self._index_offset += int(len(mesh.indices))
        self._vertex_offset += int(len(mesh.vertices))

    def build(self) -> List[DrawCommand]:
        return list(self._commands)


__all__ = ["DrawCommand", "MultiDrawIndirectBatch"]

