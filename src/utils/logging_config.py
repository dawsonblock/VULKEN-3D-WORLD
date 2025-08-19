from __future__ import annotations

import logging
import os


def configure_logging() -> None:
    """Configure basic logging for the project."""
    level_name = os.getenv("LOG_LEVEL", "INFO").upper()
    level = getattr(logging, level_name, logging.INFO)
    logging.basicConfig(level=level, format="%(levelname)s: %(message)s")

