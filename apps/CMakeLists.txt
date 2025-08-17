cmake_minimum_required(VERSION 3.10)
project(Apps)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
# Try to locate GLU component
find_package(OpenGL COMPONENTS GLU)
if(NOT TARGET OpenGL::GLU)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(OPENGL_GLU REQUIRED IMPORTED_TARGET glu)
  add_library(OpenGL::GLU ALIAS PkgConfig::OPENGL_GLU)
endif()
find_package(glfw3 REQUIRED)

# --- Build tagging injection ---
set(VOXELVK_BRANCH_SOURCES "project_fixed + VoxelVK_Elite_ALL_release_ready + voxel_upgrades_patch + project_compute_graphics_chain + project_dynamic_pbr + project_next_tier_all")
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/GenBuildInfo.cmake")
  include(${CMAKE_SOURCE_DIR}/cmake/GenBuildInfo.cmake)
  include_directories(${CMAKE_BINARY_DIR}/generated)
endif()


add_executable(smoke_headless smoke_headless.cpp)
set_target_properties(smoke_headless PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(smoke_headless PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(smoke_headless PRIVATE Vulkan::Vulkan)

add_executable(smoke_graphics_headless smoke_graphics_headless.cpp)
set_target_properties(smoke_graphics_headless PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(smoke_graphics_headless PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(smoke_graphics_headless PRIVATE Vulkan::Vulkan)

# Viewer application demonstrating Vulkan with GLFW
add_executable(viewer demo_window.cpp)
set_target_properties(viewer PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(viewer PRIVATE ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(viewer PRIVATE Vulkan::Vulkan glfw OpenGL::GLU)

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

add_custom_target(mirror_spv_ibl ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/assets/spv/ibl
  COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_BINARY_DIR}/spv/ibl/brdf_lut.comp.spv
          ${CMAKE_BINARY_DIR}/assets/spv/ibl/brdf_lut.comp.spv
  DEPENDS VoxelVK_Shaders)

if(TARGET VoxelVK_Elite_ALL)
  add_dependencies(VoxelVK_Elite_ALL mirror_spv_ibl)
endif()
