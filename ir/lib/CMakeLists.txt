include_directories(${shizuku_SOURCE_DIR}/ir/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/ir/folded)
include_directories(${shizuku_SOURCE_DIR}/ir/reader)
include_directories(${shizuku_SOURCE_DIR}/ir/flatten)

add_custom_command(
    OUTPUT ir_reader_yacc.cc ir_reader_def.h
    COMMAND bison -o ir_reader_yacc.cc --defines=ir_reader_def.h
                     -Wconflicts-sr -Wcounterexamples
                     ${shizuku_SOURCE_DIR}/ir/reader/ir_reader.y
    DEPENDS ${shizuku_SOURCE_DIR}/ir/reader/ir_reader.y
    COMMENT "Generating ir_reader_yacc.cc"
)

add_custom_command(
    OUTPUT ir_reader_lex.cc
    COMMAND flex -o ir_reader_lex.cc ${shizuku_SOURCE_DIR}/ir/reader/ir_reader.l
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ir_reader_def.h
    DEPENDS ${shizuku_SOURCE_DIR}/ir/reader/ir_reader.l
    COMMENT "Generating ir_reader_lex.cc"
)

file(GLOB ir_folded_files "${shizuku_SOURCE_DIR}/ir/folded/*.cc")
file(GLOB ir_flatten_files "${shizuku_SOURCE_DIR}/ir/flatten/*.cc")
file(GLOB ir_reader_files "${shizuku_SOURCE_DIR}/ir/reader/*.cc")
add_library(ir ${ir_folded_files}
                    ${ir_flatten_files}
                    ${CMAKE_CURRENT_BINARY_DIR}/ir_reader_lex.cc
                    ${CMAKE_CURRENT_BINARY_DIR}/ir_reader_yacc.cc
                    ${ir_reader_files})
