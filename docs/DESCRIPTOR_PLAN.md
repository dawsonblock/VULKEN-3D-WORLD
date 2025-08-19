# Descriptor Set Plan

This engine uses a single persistent descriptor set to bind long-lived resources.  The layout is
shared across graphics and compute passes so that material and lighting data remain available
throughout a frame.  Binding numbers are fixed to simplify shader includes.

| Binding | Type | Resource | Notes |
|--------:|------|----------|-------|
| **0** | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | Camera & projection matrices | Updated every frame |
| **1** | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` (array) | Material textures | Descriptor indexing, array size 32 |
| **2** | `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` | Material parameters buffer | Indexed by material ID |
| **3** | `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | Light data (directional & point) | |
| **4** | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | Environment cubemap for IBL | Prefiltered radiance map |
| **5** | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | Cascaded shadow map array | `sampler2DArray`, PCF/PCSS friendly |
| **6** | `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | BRDF integration LUT | Used by PBR/IBL shaders |

The descriptor pool allocates one set with sufficient counts for the above resources.  Material
textures reserve 32 slots to allow sparse indexing.  Additional transient sets (e.g. post‐process or
compute passes) should use separate pools to avoid fragmentation.

