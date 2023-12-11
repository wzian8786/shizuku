include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/vid/private)
include_directories(${shizuku_SOURCE_DIR}/util/public)

file(GLOB vid_files "${shizuku_SOURCE_DIR}/vid/private/*.cc")

add_library(vid ${vid_files})
