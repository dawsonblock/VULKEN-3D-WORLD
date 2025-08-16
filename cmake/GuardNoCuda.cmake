if(VOXELVK_ENABLE_CUDA)
  return()
endif()

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_QUIET TRUE)
try_compile(CUDA_INCLUDES_FOUND
  "${CMAKE_BINARY_DIR}/cmake_cuda_check"
  SOURCES "${CMAKE_CURRENT_LIST_DIR}/try_no_cuda_guard.cpp")
if(CUDA_INCLUDES_FOUND)
  message(FATAL_ERROR "CUDA headers found while VOXELVK_ENABLE_CUDA=OFF.\n"
    "CMAKE_CUDA_COMPILER=${CMAKE_CUDA_COMPILER}\n"
    "ENV{CUDA_PATH}=$ENV{CUDA_PATH} ENV{CUDA_HOME}=$ENV{CUDA_HOME}\n"
    "Unset these variables or enable CUDA with -DVOXELVK_ENABLE_CUDA=ON.")
endif()
