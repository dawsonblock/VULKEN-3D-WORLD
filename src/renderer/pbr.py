from __future__ import annotations
import numpy as np
from typing import Tuple
from .resource_manager import resources

# Retrieve precomputed environment maps for binding in the lighting shader.
def bind_pbr_textures() -> Tuple[np.ndarray, np.ndarray]:
    irr = resources.get_texture("irradiance").data
    pre = resources.get_texture("prefilter").data
    return irr, pre
