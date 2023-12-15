include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/netlist/public)

add_executable(shizuku shizuku.cc)
target_link_libraries(shizuku netlist
                              vid
                              util 
                              -pthread)
