include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/runtime/public)

file(GLOB runtime_test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(runtime_tests ${runtime_test_files})
target_link_libraries(runtime_tests runtime ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(run_runtime_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/runtime_tests)
