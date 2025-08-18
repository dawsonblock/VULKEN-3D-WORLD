
# GuardNoCuda.cmake - ensures CUDA is not inadvertently pulled in when disabled

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

set(_cuda_vars CUDA_TOOLKIT_ROOT_DIR CUDAToolkit_ROOT)
foreach(v IN LISTS _cuda_vars)
  if(DEFINED ${v})
    message(WARNING "CUDA variable '${v}' is set while VOXELVK_ENABLE_CUDA=OFF. Unset it to silence this warning.")
  endif()
endforeach()

try_compile(CUDA_COMPILES
  "${CMAKE_BINARY_DIR}/cuda_guard"
  "${CMAKE_CURRENT_LIST_DIR}/try_no_cuda_guard.cpp"
  CMAKE_FLAGS -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
  OUTPUT_VARIABLE CUDA_GUARD_LOG
)

if(CUDA_COMPILES)
  message(FATAL_ERROR "CUDA detected but VOXELVK_ENABLE_CUDA=OFF. Set VOXELVK_ENABLE_CUDA=ON or remove CUDA from your environment.\n${CUDA_GUARD_LOG}")
else()
  message(STATUS "CUDA not detected (as expected)")

endif()
