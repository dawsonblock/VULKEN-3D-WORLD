file(REMOVE_RECURSE
  "CMakeFiles/VoxelVK_Shaders"
  "spv/ai/comp/blockize.spv"
  "spv/common/vert/fullscreen.spv"
  "spv/comp/diff_voxelize.spv"
  "spv/comp/frustum_cull.spv"
  "spv/comp/greedy_meshing.spv"
  "spv/comp/sdf_jump_flood.spv"
  "spv/comp/voxel_fill.spv"
  "spv/comp/voxelize_sat.spv"
  "spv/ibl/comp/brdf_lut.spv"
  "spv/post/frag/atmosphere.spv"
  "spv/post/frag/clouds.spv"
  "spv/shadows/frag/csm_depth.spv"
  "spv/shadows/vert/csm_depth.spv"
  "spv/voxelize/frag/voxelize.spv"
  "spv/voxelize/vert/voxelize.spv"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/VoxelVK_Shaders.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
