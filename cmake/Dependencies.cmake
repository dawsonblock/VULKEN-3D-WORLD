include(FetchContent)

FetchContent_Declare(
  VulkanMemoryAllocator
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.1.0
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

add_library(VMA INTERFACE)
target_include_directories(VMA INTERFACE ${vulkanmemoryallocator_SOURCE_DIR}/include)
