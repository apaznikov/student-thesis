#pragma once

#include <cstdint>

namespace bb::arch {

namespace {

template <int TIndex>
static constexpr std::uint8_t get_byte_le(std::uint64_t v) {
  constexpr auto shift = 8 * TIndex;
  std::uint64_t mask = UINT64_C(0xff) << shift;
  return (v & mask) >> shift;
}

}  // namespace

static inline void gen_indirect_jmp_to(std::uint8_t* memory,
                                       std::uintptr_t addr) {
  // movabs <addr>, %rax
  *(memory++) = 0x48;
  *(memory++) = 0xb8;
  *(memory++) = get_byte_le<0>(addr);
  *(memory++) = get_byte_le<1>(addr);
  *(memory++) = get_byte_le<2>(addr);
  *(memory++) = get_byte_le<3>(addr);
  *(memory++) = get_byte_le<4>(addr);
  *(memory++) = get_byte_le<5>(addr);
  *(memory++) = get_byte_le<6>(addr);
  *(memory++) = get_byte_le<7>(addr);

  // jmpq *%rax
  *(memory++) = 0xff;
  *(memory++) = 0xe0;
}

}  // namespace bb::arch
