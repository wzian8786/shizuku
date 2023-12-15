include_directories(${shizuku_SOURCE_DIR}/netlist/public)
include_directories(${shizuku_SOURCE_DIR}/vid/public)
include_directories(${shizuku_SOURCE_DIR}/util/public)
include_directories(${shizuku_SOURCE_DIR}/netlist/folded)
include_directories(${shizuku_SOURCE_DIR}/netlist/reader)
include_directories(${shizuku_SOURCE_DIR}/netlist/flatten)

add_custom_command(
    OUTPUT netlist_reader_yacc.cc netlist_reader_def.h
    COMMAND bison -o netlist_reader_yacc.cc --defines=netlist_reader_def.h
                     -Wconflicts-sr -Wcounterexamples
                     ${shizuku_SOURCE_DIR}/netlist/reader/netlist_reader.y
    DEPENDS ${shizuku_SOURCE_DIR}/netlist/reader/netlist_reader.y
    COMMENT "Generating netlist_reader_yacc.cc"
)

add_custom_command(
    OUTPUT netlist_reader_lex.cc
    COMMAND flex -o netlist_reader_lex.cc ${shizuku_SOURCE_DIR}/netlist/reader/netlist_reader.l
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/netlist_reader_def.h
    DEPENDS ${shizuku_SOURCE_DIR}/netlist/reader/netlist_reader.l
    COMMENT "Generating netlist_reader_lex.cc"
)

file(GLOB netlist_folded_files "${shizuku_SOURCE_DIR}/netlist/folded/*.cc")
file(GLOB netlist_flatten_files "${shizuku_SOURCE_DIR}/netlist/flatten/*.cc")
file(GLOB netlist_reader_files "${shizuku_SOURCE_DIR}/netlist/reader/*.cc")
add_library(netlist ${netlist_folded_files}
                    ${netlist_flatten_files}
                    ${CMAKE_CURRENT_BINARY_DIR}/netlist_reader_lex.cc
                    ${CMAKE_CURRENT_BINARY_DIR}/netlist_reader_yacc.cc
                    ${netlist_reader_files})
