find_package(Vulkan REQUIRED)

# --- Build tagging injection ---
set(VOXELVK_BRANCH_SOURCES "project_fixed + VoxelVK_Elite_ALL_release_ready + voxel_upgrades_patch + project_compute_graphics_chain + project_dynamic_pbr + project_next_tier_all")
include(${CMAKE_SOURCE_DIR}/cmake/GenBuildInfo.cmake)
include_directories(${CMAKE_BINARY_DIR}/generated)


add_executable(smoke_headless smoke_headless.cpp)
set_target_properties(smoke_headless PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(smoke_headless PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(smoke_headless PRIVATE Vulkan::Vulkan)

add_executable(smoke_graphics_headless smoke_graphics_headless.cpp)
set_target_properties(smoke_graphics_headless PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(smoke_graphics_headless PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(smoke_graphics_headless PRIVATE Vulkan::Vulkan)

# --- Build tagging sources ---
list(APPEND VOXELVK_ELITE_SOURCES
  src/core/build_info_print.cpp
  src/core/runtime_debug.cpp
  src/core/gpu_compat.cpp)

# --- CSM integration sources ---
list(APPEND VOXELVK_ELITE_SOURCES
  src/render/csm_cpu.cpp
  src/render/csm_pass.cpp)

# --- CSM descriptor layout helper ---
list(APPEND VOXELVK_ELITE_SOURCES src/render/csm_layout.cpp)
list(APPEND VOXELVK_ELITE_SOURCES src/render/pbr_lighting_pass.cpp)

if(TARGET VoxelVK_Elite_ALL)
  target_sources(VoxelVK_Elite_ALL PRIVATE ${VOXELVK_ELITE_SOURCES})
endif()

# --- BRDF LUT compute integration ---
if(TARGET VoxelVK_Elite_ALL)
  set(VOXELVK_ELITE_SOURCES)
  list(APPEND VOXELVK_ELITE_SOURCES src/render/ibl_brdf.cpp)
  target_sources(VoxelVK_Elite_ALL PRIVATE ${VOXELVK_ELITE_SOURCES})
  target_include_directories(VoxelVK_Elite_ALL PRIVATE ${CMAKE_SOURCE_DIR}/external/vma)
endif()

# --- SDF generator staging pipeline ---
if(TARGET VoxelVK_Elite_ALL)
  set(VOXELVK_ELITE_SOURCES)
  list(APPEND VOXELVK_ELITE_SOURCES src/render/sdf_generator.cpp)
  target_sources(VoxelVK_Elite_ALL PRIVATE ${VOXELVK_ELITE_SOURCES})
  target_include_directories(VoxelVK_Elite_ALL PRIVATE ${CMAKE_SOURCE_DIR}/external/vma)
endif()

add_custom_target(mirror_spv_ibl ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets/spv/ibl
  COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_BINARY_DIR}/spv/ibl/brdf_lut.comp.spv
          ${CMAKE_BINARY_DIR}/assets/spv/ibl/brdf_lut.comp.spv
  DEPENDS VoxelVK_Shaders)

if(TARGET VoxelVK_Elite_ALL)
  add_dependencies(VoxelVK_Elite_ALL mirror_spv_ibl)
endif()
