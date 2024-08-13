#pragma once

#include <bit>
#include <cassert>
#include <cstdint>

#include "arch/instr.hpp"

namespace bb::arch {

[[gnu::always_inline]] inline std::uintptr_t GetReturnAddr() {
  // Не используется в RISC-V
  assert(false && "BUG: The function should not be called");
  return 0;
}

static inline void InsertNops(InstrData* start, std::size_t total_size) {
  assert((total_size % 2) == 0 && "Align should be % by 2");

  // В RISC-V NOP равен инструкции addi x0, x0, 0. Что в сущности 0x0000'0013.
  // Не забываем про порядок байтов (LE должен быть).
  for (std::size_t i = 0; i < total_size / 2; ++i) {
    *start++ = 0x0010;
  }
}

}  // namespace bb::arch
