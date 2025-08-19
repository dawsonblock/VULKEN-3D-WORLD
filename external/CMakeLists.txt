include(FetchContent)

FetchContent_Declare(
  zstd
  URL https://github.com/facebook/zstd/releases/download/v1.5.5/zstd-1.5.5.tar.gz
  SOURCE_SUBDIR build/cmake
)
FetchContent_MakeAvailable(zstd)
add_library(ZSTD::ZSTD ALIAS libzstd_static)

FetchContent_Declare(
  lz4
  URL https://github.com/lz4/lz4/archive/refs/tags/v1.9.4.tar.gz
  SOURCE_SUBDIR build/cmake
)
FetchContent_MakeAvailable(lz4)
add_library(LZ4::lz4 ALIAS lz4_static)
