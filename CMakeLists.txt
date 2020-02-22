cmake_minimum_required(VERSION 3.4)
set(CMAKE_CXX_STANDARD 17)

project(fuzilly)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

execute_process(
  COMMAND llvm-config-9 --src-root
  RESULT_VARIABLE HAD_ERROR
  OUTPUT_VARIABLE LLVM_ROOT
)

if (NOT HAD_ERROR)
  string(REGEX REPLACE "[ \t]*[\r\n]+[ \t]*" "" LLVM_ROOT ${LLVM_ROOT})
else()
  message(FATAL_ERROR "llvm-config-9 failed -- ${HAD_ERROR}")
endif()

# A bit of a sanity check:
if (NOT EXISTS ${LLVM_ROOT}/include/llvm)
  message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is invalid")
endif()

list(APPEND CMAKE_MODULE_PATH "${LLVM_ROOT}/lib/cmake/llvm")
list(APPEND CMAKE_PREFIX_PATH "${LLVM_ROOT}/lib/cmake/llvm")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(LLVM REQUIRED CONFIG)

include(HandleLLVMOptions) # load  additional  config
include(AddLLVM) # used to add  our  own  modules

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(FZL_COMMON_SOURCES "${PROJECT_SOURCE_DIR}/common/fzl.cpp")

include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories("${PROJECT_SOURCE_DIR}/common/")

add_subdirectory(runtime)
add_subdirectory(passes)
add_subdirectory(tools)

######################################### TESTS
# https://github.com/quarkslab/llvm-dev-meeting-tutorial-2015/blob/master/CMakeLists.txt
# Find python
include(FindPythonModule)
set(Python_ADDITIONAL_VERSIONS "3")
find_package(PythonInterp)
find_python_module(lit REQUIRED)

# Find FileCheck
if(NOT EXISTS ${LLVM_TOOLS_BINARY_DIR}/FileCheck)
  message(FATAL_ERROR "LLVM wasn't configured with -DLLVM_INSTALL_UTILS, cannot use FileCheck")
endif()

# lit configuration and target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/llvm-tests/lit.cfg.in" "${CMAKE_CURRENT_BINARY_DIR}/llvm-tests/lit.cfg")
add_custom_target(check
        COMMAND ${PYTHON_EXECUTABLE} -m lit.main
                "${CMAKE_CURRENT_BINARY_DIR}/llvm-tests" -v
        DEPENDS BasicBlockTaggerPass BasicBlockInstrPass fuzzilly fuzzilly-retrace
)
