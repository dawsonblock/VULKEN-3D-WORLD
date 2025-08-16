
"""Image based lighting utilities.

This module exposes a small Python wrapper around the native
``voxelvk::CreateBRDFLUT`` routine.  The routine builds the split-sum
BRDF lookup texture used for physically based rendering (PBR).  The
Python wrapper defers to the C++ implementation via the :mod:`voxelvk`
Python bindings.

The binding is optional – if the module cannot be imported a
``RuntimeError`` is raised when ``build_brdf_lut`` is invoked.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any


@dataclass
class BRDFLUT:
    """Container mirroring ``voxelvk::BRDFLUT``.

    Attributes
    ----------
    image, view, sampler, allocation:
        Integer handles to the Vulkan image resources created by the
        native function.
    width, height:
        Dimensions of the square lookup texture.
    format:
        Vulkan format of the LUT image, typically ``VK_FORMAT_R16G16_SFLOAT``.
    """

    image: int
    view: int
    sampler: int
    allocation: int
    width: int
    height: int
    format: int


def build_brdf_lut(device: Any, allocator: Any, size: int = 256) -> BRDFLUT:
    """Create a BRDF lookup texture using the native implementation.

    Parameters
    ----------
    device:
        Vulkan ``VkDevice`` handle passed through to the native
        ``CreateBRDFLUT`` function.
    allocator:
        ``VmaAllocator`` (or backend specific allocator) used for the
        LUT's image memory.
    size:
        Width and height of the square LUT texture.  Defaults to 256.

    Returns
    -------
    BRDFLUT
        Dataclass containing the handles returned by the native code.

    Raises
    ------
    RuntimeError
        If the ``voxelvk`` bindings are not available or the native
        function fails.
    """

    try:  # Import on demand to keep module import cheap when bindings missing
        from voxelvk import ibl_brdf as _ibl_brdf  # type: ignore
        codex/implement-c++-createbrdflut-binding-sil6uv
    except (ImportError, ModuleNotFoundError) as exc:  # pragma: no cover - defensive path
    except Exception as exc:  # pragma: no cover - defensive path
        codex/enhance-superbuild-cmake-configuration
        raise RuntimeError(
            "voxelvk bindings are required to build the BRDF LUT"
        ) from exc

    native_lut = _ibl_brdf.BRDFLUT()
    # The binding mirrors the C++ signature:
    # success = CreateBRDFLUT(device, allocator, out, size)
    success = _ibl_brdf.CreateBRDFLUT(device, allocator, native_lut, size)
    if not success:  # pragma: no cover - defensive path
        raise RuntimeError("CreateBRDFLUT returned failure")

    return BRDFLUT(
        image=native_lut.image,
        view=native_lut.view,
        sampler=native_lut.sampler,
        allocation=native_lut.allocation,
        width=native_lut.width,
        height=native_lut.height,
        format=native_lut.format,
    )
