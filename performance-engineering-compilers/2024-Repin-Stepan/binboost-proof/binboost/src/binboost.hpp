#pragma once

#include <cstdint>

extern "C" {
[[gnu::noinline]] std::uintptr_t bb_jit_begin_impl(
    std::uintptr_t opt_code_start);
}
