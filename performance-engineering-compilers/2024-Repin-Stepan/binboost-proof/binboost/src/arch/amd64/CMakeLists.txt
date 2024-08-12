add_library(
    bb_arch_amd64
        trampoline.cpp
)

target_include_directories(
    bb_arch_amd64
        PUBLIC include/
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=riscv64g")
