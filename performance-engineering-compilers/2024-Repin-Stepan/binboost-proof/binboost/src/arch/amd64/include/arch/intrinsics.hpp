#pragma once

#include <bit>
#include <cstdint>

namespace bb::arch {

// @todo move to common arch
[[gnu::always_inline]] inline std::uintptr_t GetReturnAddr() {
  // Не получается в x86 получать return address в trampoline. Поэтому
  // использую __builtin_return_address(), который у меня отрабатывает для
  // Debug и Release. В дальнейшем стоит рассмотреть использование функции
  // backtrace.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"
  return std::bit_cast<std::uintptr_t>(__builtin_return_address(1));
#pragma GCC diagnostic pop
}

static inline bool IsCallInstrToAddr(const std::uint8_t* code,
                                     std::uintptr_t addr) {
  if (*code != 0xe9 && *code != 0xe8) {
    return false;
  }

  static constexpr auto kCallInstrSize = 5;

  auto read_uint32 = [](const std::uint8_t* ptr) {
    return *reinterpret_cast<const std::uint32_t*>(ptr);
  };
  std::uint32_t rel_addr = read_uint32(code + 1);
  auto next_instr_addr =
      reinterpret_cast<std::uintptr_t>(code + kCallInstrSize);

  return (next_instr_addr + rel_addr) == addr;
}

static inline std::uintptr_t FindCallInstrTo(std::uintptr_t start,
                                             std::uintptr_t addr) {
  static constexpr int kMaxCodeSizeInBytes = 512;

  const auto* code_ptr = reinterpret_cast<const std::uint8_t*>(start);
  int iteration = 0;

  while (!IsCallInstrToAddr(code_ptr, addr)) {
    ++code_ptr;
    ++iteration;
    if (iteration >= kMaxCodeSizeInBytes) {
      return 0;
    }
  }
  return std::bit_cast<std::uintptr_t>(code_ptr) - 1;
}

}  // namespace bb::arch
