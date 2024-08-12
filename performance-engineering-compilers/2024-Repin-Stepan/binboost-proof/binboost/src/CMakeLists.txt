add_library(
    bb_hint
        binboost.cpp
        basic_block_parser.cpp
        code_analyzer.cpp
)
target_include_directories(
    bb_hint
        PUBLIC ../include/
)

if(BB_ARCH STREQUAL "riscv")
    add_subdirectory(arch/riscv)
    target_link_libraries(bb_hint bb_arch_riscv)
elseif(BB_ARCH STREQUAL "amd64")
    add_subdirectory(arch/amd64)
    target_link_libraries(bb_hint bb_arch_amd64)
else()
    message(FATAL_ERROR "Unknown CPU architecture: '${BB_ARCH}'")
endif()

add_library(bb_hint_priv INTERFACE)
target_include_directories(bb_hint_priv INTERFACE ./)
target_link_libraries(bb_hint_priv INTERFACE bb_hint)
