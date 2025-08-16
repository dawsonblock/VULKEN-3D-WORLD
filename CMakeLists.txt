cmake_minimum_required(VERSION 3.24...3.29)
cmake_policy(SET CMP0091 NEW) # MSVC runtime library flags
cmake_policy(SET CMP0048 NEW) # project version
cmake_policy(SET CMP0077 NEW) # option() honors normal variables
project(SUPERBUILD_VULKEN_CSM LANGUAGES CXX)

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

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wdouble-promotion -Wformat=2 -Wno-unused-parameter)
  add_link_options(-Wl,--no-undefined)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/GuardNoCuda.cmake)

find_package(Vulkan QUIET)
if(NOT Vulkan_FOUND)
  message(WARNING "Vulkan SDK not found. Vulkan targets will be skipped. Install from https://vulkan.lunarg.com/")
endif()

# --- Build tagging injection ---
set(VOXELVK_BRANCH_SOURCES "project_fixed + VoxelVK_Elite_ALL_release_ready + voxel_upgrades_patch + project_compute_graphics_chain + project_dynamic_pbr + project_next_tier_all")
include(${CMAKE_SOURCE_DIR}/cmake/GenBuildInfo.cmake)
include_directories(${CMAKE_BINARY_DIR}/generated)

if(Vulkan_FOUND)
  find_package(glfw3 QUIET)
  if(glfw3_FOUND)
    set(GLFW_LIB glfw)
  else()
    find_package(GLFW REQUIRED)
    set(GLFW_LIB GLFW::GLFW)
  endif()
  find_package(Threads REQUIRED)
endif()

if(Vulkan_FOUND)
  # Shader pipeline (glslc preferred; fallback glslangValidator)
  find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" "/usr/bin" "/usr/local/bin")
  if(NOT GLSLC_EXECUTABLE)
    find_program(GLSLANG_VALIDATOR NAMES glslangValidator HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" "/usr/bin" "/usr/local/bin")
    if(NOT GLSLANG_VALIDATOR)
      message(FATAL_ERROR "No shader compiler found (glslc/glslangValidator). Install Vulkan SDK.")
    endif()
    set(SHADER_COMPILER ${GLSLANG_VALIDATOR})
    set(SHADER_FLAGS -V --target-env vulkan1.3 -g)
    set(SHADER_DEPFLAGS -MMD -MF)
  else()
    set(SHADER_COMPILER ${GLSLC_EXECUTABLE})
    set(SHADER_FLAGS --target-env=vulkan1.3 -O -g)
    set(SHADER_DEPFLAGS -MD -MF)
  endif()

  file(GLOB_RECURSE GLSL_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/shaders_vk/*.*.glsl")
  set(SPV_DIR "${CMAKE_BINARY_DIR}/spv")

  function(compile_glsl OUT_VAR)
    set(_SPVS "")
    foreach(GLSL_FILE IN LISTS ARGN)
      get_filename_component(FILE_NAME ${GLSL_FILE} NAME_WE)
      get_filename_component(FILE_DIR  ${GLSL_FILE} DIRECTORY)
      file(RELATIVE_PATH REL_DIR "${CMAKE_SOURCE_DIR}/shaders_vk" "${FILE_DIR}")
      set(OUT_DIR "${SPV_DIR}/${REL_DIR}")
      file(MAKE_DIRECTORY "${OUT_DIR}")
      set(SPV_FILE "${OUT_DIR}/${FILE_NAME}.spv")
      set(DEP_FILE "${SPV_FILE}.d")
      add_custom_command(OUTPUT ${SPV_FILE}
        COMMAND ${SHADER_COMPILER} ${SHADER_FLAGS} ${SHADER_DEPFLAGS} ${DEP_FILE} -o ${SPV_FILE} ${GLSL_FILE}
        DEPFILE ${DEP_FILE}
        DEPENDS ${GLSL_FILE}
        COMMENT "Compiling ${REL_DIR}/${FILE_NAME}.glsl")
      list(APPEND _SPVS ${SPV_FILE})
    endforeach()
    set(${OUT_VAR} ${_SPVS} PARENT_SCOPE)
  endfunction()

  compile_glsl(SPV_OUT ${GLSL_SOURCES})
  add_custom_target(VoxelVK_Shaders ALL DEPENDS ${SPV_OUT})
endif()

if(Vulkan_FOUND)
  # Targets (smoke only; plug real engine files as needed)
  add_executable(smoke_headless apps/smoke_headless.cpp)
  add_dependencies(smoke_headless VoxelVK_Shaders)
  target_include_directories(smoke_headless PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(smoke_headless PRIVATE Vulkan::Vulkan Threads::Threads ${GLFW_LIB})
  target_compile_definitions(smoke_headless PRIVATE
    $<$<BOOL:${ENABLE_VALIDATION_LAYERS}>:ENABLE_VALIDATION_LAYERS=1>
    $<$<BOOL:${ENABLE_IMGUI_OVERLAY}>:ENABLE_IMGUI_OVERLAY=1>)

  # CUDA/TRT guards if you later add GPU/AI targets
  target_compile_definitions(smoke_headless PRIVATE
    $<$<BOOL:${VOXELVK_ENABLE_CUDA}>:VOXELVK_ENABLE_CUDA=1>
    $<$<BOOL:${VOXELVK_ENABLE_TENSORRT}>:VOXELVK_ENABLE_TENSORRT=1>)
endif()

include(CTest)
enable_testing()

add_subdirectory(tools/ipc_smoke_test)

if(ENABLE_TESTS)
  if(Vulkan_FOUND)
    add_test(NAME smoke_headless COMMAND smoke_headless)
    set_tests_properties(smoke_headless PROPERTIES TIMEOUT 30)
  endif()
  add_test(NAME ipc_smoke_test COMMAND ipc_smoke_test)
endif()

add_custom_target(superbuild_check
  COMMAND ${CMAKE_COMMAND} -E echo "C++ compiler: ${CMAKE_CXX_COMPILER}"
  COMMAND ${CMAKE_COMMAND} -E echo "Vulkan: ${Vulkan_FOUND}"
  COMMAND ${CMAKE_COMMAND} -E echo "CUDA enabled: ${VOXELVK_ENABLE_CUDA}"
  COMMAND ${CMAKE_COMMAND} -E echo "TensorRT enabled: ${VOXELVK_ENABLE_TENSORRT}")

message(STATUS "=== Build Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(Vulkan_FOUND)
  message(STATUS "Shader Compiler: ${SHADER_COMPILER}")
endif()
message(STATUS "=====================")

# --- Build tagging sources ---
if(TARGET VoxelVK_Elite_ALL)
  target_sources(VoxelVK_Elite_ALL PRIVATE
    src/core/build_info_print.cpp
    src/core/runtime_debug.cpp
    src/core/gpu_compat.cpp)
endif()

# --- CSM+PCSS integration sources ---
if(TARGET VoxelVK_Elite_ALL)
  target_sources(VoxelVK_Elite_ALL PRIVATE src/render/csm_pass.cpp)
endif()

# --- CSM wiring helpers ---
if(TARGET VoxelVK_Elite_ALL)
  target_sources(VoxelVK_Elite_ALL PRIVATE src/render/csm_cpu.cpp src/render/csm_pass.cpp)
endif()

# --- CSM descriptor layout helper ---
if(TARGET VoxelVK_Elite_ALL)
  target_sources(VoxelVK_Elite_ALL PRIVATE src/render/csm_layout.cpp)
endif()

# Optional external projects
if(EXISTS "${CMAKE_SOURCE_DIR}/external/VULKEN-3D-WORLD/CMakeLists.txt")
  add_subdirectory(external/VULKEN-3D-WORLD EXCLUDE_FROM_ALL)
else()
  message(WARNING "external/VULKEN-3D-WORLD missing. Clone with: gh repo clone VULKEN-3D/VULKEN-3D-WORLD external/VULKEN-3D-WORLD")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/external/CSM_SNAPPED/CMakeLists.txt")
  add_subdirectory(external/CSM_SNAPPED EXCLUDE_FROM_ALL)
else()
  message(WARNING "external/CSM_SNAPPED missing. Ensure snapshot present under external/CSM_SNAPPED")
endif()
