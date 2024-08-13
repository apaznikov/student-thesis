#pragma once

#include <cstdint>

namespace bb::arch {

enum class InstrFlags {
  Jump,
};

struct InstrBase {
  std::uint16_t offset;
  std::uint8_t size;
  std::uint8_t flags;
};

} //
