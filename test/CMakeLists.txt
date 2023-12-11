find_package(Boost REQUIRED 
                   COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(util)
add_subdirectory(vid)
add_subdirectory(runtime)
add_subdirectory(netlist)
