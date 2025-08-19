"""Async I/O utilities.

Provides a simple wrapper around :class:`ThreadPoolExecutor` that can be
used as a context manager. The wrapper exposes just the ``submit`` and
``shutdown`` methods to limit the surface area used by callers.
"""

from __future__ import annotations

from concurrent.futures import Future, ThreadPoolExecutor
from typing import Any, Callable


class AsyncIOPool:
    """Thin wrapper over :class:`ThreadPoolExecutor`.

    Parameters
    ----------
    max_workers:
        Maximum number of worker threads. Defaults to ``4``.
    """

    def __init__(self, max_workers: int = 4) -> None:
        self._executor = ThreadPoolExecutor(max_workers=max_workers)

    def submit(self, fn: Callable[..., Any], *args: Any, **kwargs: Any) -> Future:
        """Submit ``fn`` to the underlying executor."""
        return self._executor.submit(fn, *args, **kwargs)

    def shutdown(self, wait: bool = True) -> None:
        """Shutdown the underlying executor."""
        self._executor.shutdown(wait=wait)

    # ------------------------------------------------------------------
    # Context manager protocol
    # ------------------------------------------------------------------
    def __enter__(self) -> "AsyncIOPool":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:  # pragma: no cover - trivial
        self.shutdown()
