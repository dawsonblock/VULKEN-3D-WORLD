cmake_minimum_required(VERSION 3.24...3.29)

# Modern policies for deterministic behaviour
cmake_policy(SET CMP0091 NEW) # MSVC runtime library selection
cmake_policy(SET CMP0077 NEW) # option() honours normal variables
cmake_policy(SET CMP0126 NEW) # cmake -E env behavior

project(VoxelVK_Elite_ALL LANGUAGES CXX)

add_library(VoxelVK_Elite_ALL STATIC
  # List your source files here, for example:
  src/main.cpp
  src/voxel.cpp
  src/vk_elite.cpp
)
# Ensure headers under src/ are discoverable for includes like "world/svo.hpp"
target_include_directories(VoxelVK_Elite_ALL PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/third_party/vma)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_VALIDATION_LAYERS "Enable Vulkan validation layers" ON)
option(ENABLE_VK_DEBUG_MARKERS  "Enable VK_EXT_debug_utils markers" ON)
option(ENABLE_IMGUI_OVERLAY     "Enable ImGui debug HUD overlay" ON)
option(ENABLE_TESTS             "Enable CTest targets" ON)
option(VOXELVK_ENABLE_CUDA      "Enable CUDA acceleration" OFF)
option(VOXELVK_ENABLE_TENSORRT  "Enable TensorRT (requires CUDA)" OFF)
option(ENABLE_VULKAN            "Build Vulkan dependent targets" ON)
option(USE_VMA_SHIM             "Use bundled minimal VMA shim (vk_mem_alloc.h)" ON)

if(NOT VOXELVK_ENABLE_CUDA)
  include(cmake/GuardNoCuda.cmake)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wdouble-promotion -Wformat=2 -Wno-unused-parameter)
  add_link_options(-Wl,--no-undefined)
endif()

# ---- Toolchain diagnostics ----
add_custom_target(superbuild_check
  COMMAND ${CMAKE_COMMAND} -E echo "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}"
  COMMAND ${CMAKE_COMMAND} -E echo "Build type : ${CMAKE_BUILD_TYPE}"
  COMMAND ${CMAKE_COMMAND} -E echo "CUDA       : ${VOXELVK_ENABLE_CUDA}"
  COMMAND ${CMAKE_COMMAND} -E echo "TensorRT   : ${VOXELVK_ENABLE_TENSORRT}"
)

# ---- Vulkan detection ----
if(ENABLE_VULKAN)
  find_package(Vulkan QUIET)
  if(NOT Vulkan_FOUND)
    message(WARNING "Vulkan SDK or loader not found. Vulkan targets will be skipped.\nInstall from https://vulkan.lunarg.com/sdk/home")
    set(ENABLE_VULKAN OFF)
  endif()
endif()

# --- Build tagging injection ---
set(VOXELVK_BRANCH_SOURCES "project_fixed + VoxelVK_Elite_ALL_release_ready + voxel_upgrades_patch + project_compute_graphics_chain + project_dynamic_pbr + project_next_tier_all")
# VMA include selection (shim by default)
if(USE_VMA_SHIM)
  message(STATUS "Using bundled VMA shim header")
else()
  message(STATUS "Expecting official vk_mem_alloc.h available on include path")
endif()

include(${CMAKE_SOURCE_DIR}/cmake/GenBuildInfo.cmake)
include_directories(${CMAKE_BINARY_DIR}/generated)

find_package(glfw3 QUIET)
if(glfw3_FOUND)
  set(GLFW_LIB glfw)
else()
  find_package(GLFW QUIET)
  if(GLFW_FOUND)
    set(GLFW_LIB GLFW::GLFW)
  elseif(ENABLE_VULKAN)
    message(WARNING "GLFW not found; Vulkan samples will not build")
    set(ENABLE_VULKAN OFF)
  endif()
endif()
find_package(Threads REQUIRED)

# Shader pipeline (glslc preferred; fallback glslangValidator)
set(SPV_OUT "")
if(ENABLE_VULKAN)
  find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" "/usr/bin" "/usr/local/bin")
  if(NOT GLSLC_EXECUTABLE)
    find_program(GLSLANG_VALIDATOR NAMES glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" "/usr/bin" "/usr/local/bin")
    if(GLSLANG_VALIDATOR)
      set(SHADER_COMPILER ${GLSLANG_VALIDATOR})
      set(SHADER_FLAGS -V --target-env vulkan1.3 -g)
      set(USE_GLSLC OFF)
    else()
      message(WARNING "No shader compiler found (glslc/glslangValidator). Shader compilation disabled.")
      set(ENABLE_VULKAN OFF)
    endif()
  else()
    set(SHADER_COMPILER ${GLSLC_EXECUTABLE})
    set(SHADER_FLAGS --target-env=vulkan1.3 -O -g)
    set(USE_GLSLC ON)
  endif()

  if(ENABLE_VULKAN)
    # Collect only stage-suffixed shaders (exclude utility includes)
    set(GLSL_SOURCES)
    file(GLOB_RECURSE GLSL_VERT CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders_vk/*.vert.glsl")
    file(GLOB_RECURSE GLSL_FRAG CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders_vk/*.frag.glsl")
    file(GLOB_RECURSE GLSL_COMP CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders_vk/*.comp.glsl")
    list(APPEND GLSL_SOURCES ${GLSL_VERT} ${GLSL_FRAG} ${GLSL_COMP})
    set(SPV_DIR "${CMAKE_BINARY_DIR}/spv")
    foreach(GLSL_FILE IN LISTS GLSL_SOURCES)
      get_filename_component(FILE_BASE ${GLSL_FILE} NAME)       # includes extension, e.g., csm_depth.frag.glsl
      get_filename_component(FILE_STEM ${GLSL_FILE} NAME_WE)   # e.g., csm_depth.frag
      get_filename_component(FILE_DIR  ${GLSL_FILE} DIRECTORY)
      file(RELATIVE_PATH REL_DIR "${CMAKE_SOURCE_DIR}/shaders_vk" "${FILE_DIR}")
      # Derive shader stage from suffix before .glsl
      set(STAGE "")
      if(FILE_BASE MATCHES ".*\\.vert\\.glsl$")
        set(STAGE "vert")
        string(REPLACE ".vert" "" FILE_STEM ${FILE_STEM})
      elseif(FILE_BASE MATCHES ".*\\.frag\\.glsl$")
        set(STAGE "frag")
        string(REPLACE ".frag" "" FILE_STEM ${FILE_STEM})
      elseif(FILE_BASE MATCHES ".*\\.comp\\.glsl$")
        set(STAGE "comp")
        string(REPLACE ".comp" "" FILE_STEM ${FILE_STEM})
      else()
        set(STAGE "misc")
      endif()
      set(OUT_DIR "${SPV_DIR}/${REL_DIR}/${STAGE}")
      file(MAKE_DIRECTORY "${OUT_DIR}")
      set(SPV_FILE "${OUT_DIR}/${FILE_STEM}.spv")
      set(DEP_FILE "${SPV_FILE}.d")
      if(USE_GLSLC)
        add_custom_command(OUTPUT ${SPV_FILE}
          COMMAND ${SHADER_COMPILER} ${SHADER_FLAGS} -MD -MF ${DEP_FILE} -o ${SPV_FILE} ${GLSL_FILE}
          DEPFILE ${DEP_FILE}
          DEPENDS ${GLSL_FILE}
          COMMENT "Compiling ${REL_DIR}/${FILE_BASE}")
      else()
        add_custom_command(OUTPUT ${SPV_FILE}
          COMMAND ${SHADER_COMPILER} ${SHADER_FLAGS} -o ${SPV_FILE} ${GLSL_FILE}
          DEPENDS ${GLSL_FILE}
          COMMENT "Compiling ${REL_DIR}/${FILE_BASE}")
  if(NOT TARGET vulkan_bootstrap)
    add_executable(vulkan_bootstrap apps/vulkan_bootstrap.cpp)
    target_sources(vulkan_bootstrap PRIVATE ${CMAKE_SOURCE_DIR}/src/vk/vma_helpers.cpp)
    target_include_directories(vulkan_bootstrap PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/third_party/vma)
    target_link_libraries(vulkan_bootstrap PRIVATE Vulkan::Vulkan Threads::Threads ${GLFW_LIB})
  endif()

      endif()
      list(APPEND SPV_OUT ${SPV_FILE})
    endforeach()
    add_custom_target(VoxelVK_Shaders ALL DEPENDS ${SPV_OUT})
  endif()
endif()

if(ENABLE_VULKAN)
  add_executable(smoke_headless apps/smoke_headless.cpp)
  # Shaders not required for this CPU-only smoke sample
  target_include_directories(smoke_headless PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(smoke_headless PRIVATE Vulkan::Vulkan Threads::Threads ${GLFW_LIB})
  target_compile_definitions(smoke_headless PRIVATE
    $<$<BOOL:${ENABLE_VALIDATION_LAYERS}>:ENABLE_VALIDATION_LAYERS=1>
    $<$<BOOL:${ENABLE_IMGUI_OVERLAY}>:ENABLE_IMGUI_OVERLAY=1>
    $<$<BOOL:${VOXELVK_ENABLE_CUDA}>:VOXELVK_ENABLE_CUDA=1>
    $<$<BOOL:${VOXELVK_ENABLE_TENSORRT}>:VOXELVK_ENABLE_TENSORRT=1>)
endif()

# Tests
if(ENABLE_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tools/ipc_smoke_test)
  add_test(NAME ipc_smoke_test COMMAND ipc_smoke_test)
  set_tests_properties(ipc_smoke_test PROPERTIES TIMEOUT 30)
  if(ENABLE_VULKAN)
    add_test(NAME smoke_headless COMMAND smoke_headless)
    set_tests_properties(smoke_headless PROPERTIES TIMEOUT 30)
  endif()
endif()

message(STATUS "=== Build Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Shader Compiler: ${SHADER_COMPILER}")
message(STATUS "Vulkan Enabled: ${ENABLE_VULKAN}")
message(STATUS "=====================")

# --- Build tagging sources ---
target_sources(VoxelVK_Elite_ALL PRIVATE
  src/core/build_info_print.cpp
  src/core/runtime_debug.cpp
  src/core/gpu_compat.cpp)

# --- CSM integration sources ---
target_sources(VoxelVK_Elite_ALL PRIVATE
  src/render/csm_cpu.cpp
  src/render/csm_pass.cpp)

# --- Voxelization pass ---
target_sources(VoxelVK_Elite_ALL PRIVATE src/render/voxelize_pass.cpp)

# --- CSM descriptor layout helper ---
target_sources(VoxelVK_Elite_ALL PRIVATE src/render/csm_layout.cpp)

# --- Sparse voxel octree ---
target_sources(VoxelVK_Elite_ALL PRIVATE
  src/world/svo_builder.cpp
  src/world/svo_io.cpp
  src/render/svo_traversal.cpp)

# --- Voxel SAT compute dispatch ---
# target_sources(VoxelVK_Elite_ALL PRIVATE src/render/voxelize_sat.cpp) # Removed duplicate registration
