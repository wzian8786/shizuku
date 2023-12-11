include_directories(${shizuku_SOURCE_DIR}/util/public)

file(GLOB util_test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(util_tests ${util_test_files})
target_link_libraries(util_tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                                  util
                                  -pthread)
add_test(run_util_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/util_tests)
