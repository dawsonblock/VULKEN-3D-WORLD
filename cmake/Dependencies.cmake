include(FetchContent)

# Fetch zstd
if(NOT TARGET zstd)
  FetchContent_Declare(
    zstd
    GIT_REPOSITORY https://github.com/facebook/zstd.git
    GIT_TAG v1.5.5
  )
  set(ZSTD_BUILD_PROGRAMS OFF CACHE INTERNAL "")
  set(ZSTD_BUILD_TESTS OFF CACHE INTERNAL "")
  FetchContent_MakeAvailable(zstd)
endif()

# Fetch lz4
if(NOT TARGET lz4)
  FetchContent_Declare(
    lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG v1.9.4
  )
  set(LZ4_BUILD_CLI OFF CACHE INTERNAL "")
  set(LZ4_BUILD_TESTS OFF CACHE INTERNAL "")
  FetchContent_MakeAvailable(lz4)
endif()
