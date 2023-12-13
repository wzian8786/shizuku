enable_testing()
cmake_minimum_required(VERSION 3.16)
project(shizuku)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type ${CMAKE_BUILD_TYPE}" FORCE)
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Wall -Wextra -Werror -Wno-unused-parameter")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC -Wall -Wextra -Werror -Wno-unused-parameter")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(ENABLE_TEST "Enable testing" ON)

add_subdirectory(util/lib)
add_subdirectory(vid/lib)
add_subdirectory(runtime/lib)
add_subdirectory(netlist/lib)
add_subdirectory(app)
if (ENABLE_TEST)
    add_subdirectory(test)
endif ()
