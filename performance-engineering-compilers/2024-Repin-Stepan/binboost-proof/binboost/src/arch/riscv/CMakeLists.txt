set(src_trampoline "")
if(NOT BB_EXCLUDE_TRAMPOLINE)
    set(src_trampoline trampoline.S)
endif()

add_library(
    bb_arch_riscv
        ${src_trampoline}
        decoder.cpp
)

target_include_directories(
    bb_arch_riscv
        PUBLIC include/
)
