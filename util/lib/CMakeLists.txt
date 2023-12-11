include_directories(${shizuku_SOURCE_DIR}/util/private)
include_directories(${shizuku_SOURCE_DIR}/util/public)

file(GLOB util_files "${shizuku_SOURCE_DIR}/util/private/*.cc")

add_library(util ${util_files})
