include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/ir/public)

add_definitions(-DTEST_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB ir_test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(ir_tests ${ir_test_files})
target_link_libraries(ir_tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                                  ir
                                  vid
                                  util 
                                  -pthread)

add_test(run_ir_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ir_tests)
