include_directories(${shizuku_SOURCE_DIR}/util/public)

add_executable(alloc_perf alloc_perf.cc)
target_link_libraries(util_tests util
                                 -pthread)
