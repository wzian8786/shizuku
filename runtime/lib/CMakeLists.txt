include_directories(${shizuku_SOURCE_DIR}/runtime/public)
include_directories(${shizuku_SOURCE_DIR}/runtime/private)
include_directories(${shizuku_SOURCE_DIR}/util/public)

file(GLOB runtime_files "${shizuku_SOURCE_DIR}/runtime/private/*.cc")

add_library(runtime ${runtime_files})
