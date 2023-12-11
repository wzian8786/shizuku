include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)

file(GLOB vid_test_files "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

add_executable(vid_tests ${vid_test_files})
target_link_libraries(vid_tests vid ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(run_vid_tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vid_tests)
