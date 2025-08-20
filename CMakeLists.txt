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
    else()
      message(WARNING "No shader compiler found (glslc/glslangValidator). Shader compilation disabled.")
      set(ENABLE_VULKAN OFF)
    endif()
  else()
    set(SHADER_COMPILER ${GLSLC_EXECUTABLE})
    set(SHADER_FLAGS --target-env=vulkan1.3 -O -g)
  endif()

  if(ENABLE_VULKAN)
    file(GLOB_RECURSE GLSL_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders_vk/*.glsl")
    set(SPV_DIR "${CMAKE_BINARY_DIR}/spv")
    add_compile_definitions(VOXELVK_SHADER_DIR="${SPV_DIR}")
    foreach(GLSL_FILE IN LISTS GLSL_SOURCES)
      get_filename_component(FILE_NAME ${GLSL_FILE} NAME_WE)
      get_filename_component(FILE_DIR  ${GLSL_FILE} DIRECTORY)
      file(RELATIVE_PATH REL_DIR "${CMAKE_SOURCE_DIR}/shaders_vk" "${FILE_DIR}")
      set(OUT_DIR "${SPV_DIR}/${REL_DIR}")
      file(MAKE_DIRECTORY "${OUT_DIR}")
      set(SPV_FILE "${OUT_DIR}/${FILE_NAME}.spv")
      set(DEP_FILE "${SPV_FILE}.d")
      add_custom_command(OUTPUT ${SPV_FILE}
        COMMAND ${SHADER_COMPILER} ${SHADER_FLAGS} -MD -MF ${DEP_FILE} -o ${SPV_FILE} ${GLSL_FILE}
        DEPFILE ${DEP_FILE}
        DEPENDS ${GLSL_FILE}
        COMMENT "Compiling ${REL_DIR}/${FILE_NAME}.glsl")
      list(APPEND SPV_OUT ${SPV_FILE})
    endforeach()
    add_custom_target(VoxelVK_Shaders ALL DEPENDS ${SPV_OUT})
  endif()
endif()

if(ENABLE_VULKAN)
  add_executable(smoke_headless apps/smoke_headless.cpp)
  add_dependencies(smoke_headless VoxelVK_Shaders)
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
