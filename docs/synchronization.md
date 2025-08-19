# Synchronization Helpers

The rendering passes now rely on a small helper located in
`src/render/sync_utils.hpp`. It wraps common Vulkan image and buffer
barrier code, including queue family ownership transfers.

```cpp
// Transition an image
barrierImage(cmd, image,
             oldLayout, newLayout,
             srcStage, dstStage,
             srcAccess, dstAccess,
             aspectMask);
```

Each render pass inserts the appropriate barriers before and after
executing its commands:

- **CSMShadowPass** transitions the depth atlas to
  `VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL` before rendering and makes
  depth writes visible to shaders afterwards.
- **VoxelizePass** prepares its 3D storage image for fragment shader
  writes and later transitions it for sampling.
- **SkyPass** transitions the target color image to a color attachment
  layout and back to a shader–read layout when finished.

Run with the Vulkan validation layers enabled to verify that the
expected pipeline stages and access masks match the intended usage.
