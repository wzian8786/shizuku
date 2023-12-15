include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/netlist/public)

add_definitions(-DTEST_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB netlist_test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(netlist_tests ${netlist_test_files})
target_link_libraries(netlist_tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                                  netlist
                                  vid
                                  util 
                                  -pthread)

add_test(run_netlist_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/netlist_tests)
