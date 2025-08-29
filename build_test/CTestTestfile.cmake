# CMake generated Testfile for 
# Source directory: /app
# Build directory: /app/build_test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[ipc_smoke_test]=] "/app/build_test/tools/ipc_smoke_test/ipc_smoke_test")
set_tests_properties([=[ipc_smoke_test]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "/app/CMakeLists.txt;190;add_test;/app/CMakeLists.txt;0;")
add_test([=[smoke_headless]=] "/app/build_test/smoke_headless")
set_tests_properties([=[smoke_headless]=] PROPERTIES  TIMEOUT "30" _BACKTRACE_TRIPLES "/app/CMakeLists.txt;193;add_test;/app/CMakeLists.txt;0;")
subdirs("tools/ipc_smoke_test")
