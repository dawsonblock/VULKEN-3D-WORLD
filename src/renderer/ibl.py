import numpy as np
from .resource_manager import Texture, resources

# Placeholder for BRDF LUT / cubemap build in your renderer.
def build_brdf_lut(*args, **kwargs):
    raise NotImplementedError("Implement using your rendering backend (ModernGL/OpenGL/Vulkan)." )

# Generate irradiance and prefiltered maps from an HDR skybox and store them.
def generate_environment_maps(skybox: np.ndarray) -> None:
    irradiance = skybox.mean(axis=(0, 1), keepdims=True)
    prefiltered = skybox.copy()
    resources.add_texture("irradiance", Texture(irradiance))
    resources.add_texture("prefilter", Texture(prefiltered))
