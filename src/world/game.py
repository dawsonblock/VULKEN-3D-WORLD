"""Minimal game loop integration for chunk management."""
from __future__ import annotations

from typing import Iterable

from .chunk_manager import ChunkManager


class GameWorld:
    """Thin wrapper that ties chunk management to a game loop."""

    def __init__(self, chunk_manager: ChunkManager):
        self.chunk_manager = chunk_manager

    def step(self, player_pos: Iterable[float]) -> None:
        """Advance the world one tick.

        This ensures chunks within the manager's radius around ``player_pos``
        are generated and that distant chunks are evicted.
        """

        self.chunk_manager.update(player_pos)


__all__ = ["GameWorld"]
